#pragma once
#include <memory>
#include <vector>

#include "mcpsi/context/context.h"
#include "mcpsi/context/state.h"
#include "mcpsi/cr/cr.h"
#include "mcpsi/cr/utils/ot_adapter.h"
#include "mcpsi/cr/utils/vole_adapter.h"
#include "mcpsi/ss/type.h"

namespace mcpsi {

class TrueCorrelation : public Correlation {
 private:
  bool setup_ot_{false};
  bool setup_vole_{false};  // useless

 public:
  // OT adapter
  std::shared_ptr<ot::OtAdapter> ot_sender_;
  std::shared_ptr<ot::OtAdapter> ot_receiver_;
  // Vole adapter
  std::shared_ptr<vole::VoleAdapter> vole_sender_;
  std::shared_ptr<vole::VoleAdapter> vole_receiver_;

  TrueCorrelation(std::shared_ptr<Context> ctx) : Correlation(ctx) {}

  ~TrueCorrelation() {}

  void InitOtAdapter() {
    if (setup_ot_ == true) return;

    auto conn = ctx_->GetConnection();
    if (ctx_->GetRank() == 0) {
      ot_sender_ = std::make_shared<ot::YaclSsOtAdapter>(conn->Spawn(), true);
      ot_sender_->OneTimeSetup();

      ot_receiver_ =
          std::make_shared<ot::YaclSsOtAdapter>(conn->Spawn(), false);
      ot_receiver_->OneTimeSetup();
    } else {
      ot_receiver_ =
          std::make_shared<ot::YaclSsOtAdapter>(conn->Spawn(), false);
      ot_receiver_->OneTimeSetup();

      ot_sender_ = std::make_shared<ot::YaclSsOtAdapter>(conn->Spawn(), true);
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

  void OneTimeSetup() override {
    if (setup_ot_ == true) return;
    InitOtAdapter();
    if (setup_vole_ == true) return;
    InitVoleAdapter();
  }

  internal::PTy GetKey() const override { return key_; }

  void SetKey(internal::PTy key) override {
    key_ = key;

    setup_vole_ = false;  // set it as false
    InitVoleAdapter();
    YACL_ENFORCE(setup_vole_ == true);
  }

  // entry
  void BeaverTriple(absl::Span<internal::ATy> a, absl::Span<internal::ATy> b,
                    absl::Span<internal::ATy> c) override;

  std::array<std::vector<internal::ATy>, 3> BeaverTriple(size_t num) override {
    std::vector<internal::ATy> a(num);
    std::vector<internal::ATy> b(num);
    std::vector<internal::ATy> c(num);
    BeaverTriple(absl::MakeSpan(a), absl::MakeSpan(b), absl::MakeSpan(c));
    return {a, b, c};
  }

  void AuthSet(absl::Span<const internal::PTy> in,
               absl::Span<internal::ATy> out) override;
  void AuthGet(absl::Span<internal::ATy> out) override;

  // entry
  void RandomSet(absl::Span<internal::ATy> out) override;
  void RandomGet(absl::Span<internal::ATy> out) override;
  void RandomAuth(absl::Span<internal::ATy> out) override;

  std::vector<internal::ATy> RandomSet(size_t num) override {
    std::vector<internal::ATy> ret(num);
    RandomSet(absl::MakeSpan(ret));
    return ret;
  }

  std::vector<internal::ATy> RandomGet(size_t num) override {
    std::vector<internal::ATy> ret(num);
    RandomGet(absl::MakeSpan(ret));
    return ret;
  }

  std::vector<internal::ATy> RandomAuth(size_t num) override {
    std::vector<internal::ATy> ret(num);
    RandomAuth(absl::MakeSpan(ret));
    return ret;
  }

  // entry
  void ShuffleSet(absl::Span<const size_t> perm,
                  absl::Span<internal::PTy> delta, size_t repeat = 1) override;
  void ShuffleGet(absl::Span<internal::PTy> a, absl::Span<internal::PTy> b,
                  size_t repeat = 1) override;

  std::vector<internal::PTy> ShuffleSet(absl::Span<const size_t> perm,
                                        size_t repeat = 1) override {
    std::vector<internal::PTy> delta(perm.size() * repeat);
    ShuffleSet(perm, absl::MakeSpan(delta), repeat);
    return delta;
  }

  std::pair<std::vector<internal::PTy>, std::vector<internal::PTy>> ShuffleGet(
      size_t num, size_t repeat = 1) override {
    std::vector<internal::PTy> a(num * repeat);
    std::vector<internal::PTy> b(num * repeat);
    ShuffleGet(absl::MakeSpan(a), absl::MakeSpan(b), repeat);
    return std::make_pair(a, b);
  }

 private:
  std::vector<internal::PTy> OpenAndCheck(absl::Span<const internal::ATy> in);
  // TODO:
  // internal::PTy SingleOpenAndCheck(const internal::ATy& in);
};

}  // namespace mcpsi
