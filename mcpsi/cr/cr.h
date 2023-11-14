#pragma once
#include <memory>
#include <vector>

#include "mcpsi/context/context.h"
#include "mcpsi/context/state.h"
#include "mcpsi/cr/fake_cr.h"
#include "mcpsi/cr/utils/ot_adapter.h"
#include "mcpsi/cr/utils/vole_adapter.h"
#include "mcpsi/ss/type.h"

namespace mcpsi {

// TODO: It might be better to create a "CorrelationInterface" ?
class Correlation : public State {
 private:
  std::shared_ptr<Context> ctx_;
  kFp64 key_;
  bool setup_ot_{false};
  bool setup_vole_{false};  // useless

 public:
  static const std::string id;

  // use fake correlation
  std::shared_ptr<FakeCorrelation> fake_cr_ptr_;

  // OT adapter
  std::shared_ptr<ot::OtAdapter> ot_sender_;
  std::shared_ptr<ot::OtAdapter> ot_receiver_;
  // Vole adapter
  std::shared_ptr<vole::VoleAdapter> vole_sender_;
  std::shared_ptr<vole::VoleAdapter> vole_receiver_;

  Correlation(std::shared_ptr<Context> ctx) : ctx_(ctx) {
    fake_cr_ptr_ = std::make_shared<FakeCorrelation>(ctx);
  }

  void InitOtAdapter() {
    if (setup_ot_ == true) return;

    auto conn = ctx_->GetConnection();
    if (ctx_->GetRank() == 0) {
      ot_sender_ = std::make_shared<ot::YaclKosOtAdapter>(conn->Spawn(), true);
      ot_sender_->OneTimeSetup();

      ot_receiver_ =
          std::make_shared<ot::YaclKosOtAdapter>(conn->Spawn(), false);
      ot_receiver_->OneTimeSetup();
    } else {
      ot_receiver_ =
          std::make_shared<ot::YaclKosOtAdapter>(conn->Spawn(), false);
      ot_receiver_->OneTimeSetup();

      ot_sender_ = std::make_shared<ot::YaclKosOtAdapter>(conn->Spawn(), true);
      ot_sender_->OneTimeSetup();
    }

    setup_ot_ = true;
  }

  void InitVoleAdapter() {
    YACL_ENFORCE(setup_vole_ == false);
    if (setup_ot_ == false) InitOtAdapter();

    auto conn = ctx_->GetConnection();
    if (ctx_->GetRank() == 0) {
      vole_sender_ =
          std::make_shared<vole::WolverineVoleAdapter>(conn, ot_sender_, key_);
      vole_sender_->OneTimeSetup();

      vole_receiver_ =
          std::make_shared<vole::WolverineVoleAdapter>(conn, ot_receiver_);
      vole_receiver_->OneTimeSetup();
    } else {
      vole_receiver_ =
          std::make_shared<vole::WolverineVoleAdapter>(conn, ot_receiver_);
      vole_receiver_->OneTimeSetup();

      vole_sender_ =
          std::make_shared<vole::WolverineVoleAdapter>(conn, ot_sender_, key_);
      vole_sender_->OneTimeSetup();
    }
    setup_vole_ = true;
  }

  void OneTimeSetup() {
    if (setup_ot_ == true) return;
    InitOtAdapter();
    if (setup_vole_ == true) return;
    InitVoleAdapter();
  }

  kFp64 GetKey() const { return key_; }

  void SetKey(kFp64 key) {
    key_ = key;
    fake_cr_ptr_->SetKey(key);

    setup_vole_ = false;  // set it as false
    InitVoleAdapter();
    YACL_ENFORCE(setup_vole_ == true);
  }

  // entry
  void BeaverTriple(absl::Span<internal::ATy> a, absl::Span<internal::ATy> b,
                    absl::Span<internal::ATy> c);

  std::array<std::vector<internal::ATy>, 3> inline BeaverTriple(size_t num) {
    std::vector<internal::ATy> a(num);
    std::vector<internal::ATy> b(num);
    std::vector<internal::ATy> c(num);
    BeaverTriple(absl::MakeSpan(a), absl::MakeSpan(b), absl::MakeSpan(c));
    return {a, b, c};
  }

  void AuthSet(absl::Span<const internal::PTy> in,
               absl::Span<internal::ATy> out);
  void AuthGet(absl::Span<internal::ATy> out);

  // entry
  void RandomSet(absl::Span<internal::ATy> out);
  void RandomGet(absl::Span<internal::ATy> out);
  void RandomAuth(absl::Span<internal::ATy> out);

  std::vector<internal::ATy> RandomSet(size_t num) {
    std::vector<internal::ATy> ret(num);
    RandomSet(absl::MakeSpan(ret));
    return ret;
  }

  std::vector<internal::ATy> RandomGet(size_t num) {
    std::vector<internal::ATy> ret(num);
    RandomGet(absl::MakeSpan(ret));
    return ret;
  }

  std::vector<internal::ATy> RandomAuth(size_t num) {
    std::vector<internal::ATy> ret(num);
    RandomAuth(absl::MakeSpan(ret));
    return ret;
  }

  // entry
  void ShuffleSet(absl::Span<const size_t> perm,
                  absl::Span<internal::PTy> delta);
  void ShuffleGet(absl::Span<internal::PTy> a, absl::Span<internal::PTy> b);

  std::vector<internal::PTy> ShuffleSet(absl::Span<const size_t> perm) {
    std::vector<internal::PTy> delta(perm.size());
    ShuffleSet(perm, absl::MakeSpan(delta));
    return delta;
  }

  std::pair<std::vector<internal::PTy>, std::vector<internal::PTy>> ShuffleGet(
      size_t num) {
    std::vector<internal::PTy> a(num);
    std::vector<internal::PTy> b(num);
    ShuffleGet(absl::MakeSpan(a), absl::MakeSpan(b));
    return std::make_pair(a, b);
  }
};

}  // namespace mcpsi
