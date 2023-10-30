#pragma once
#include <memory>
#include <vector>

#include "mcpsi/context/context.h"
#include "mcpsi/context/state.h"
#include "mcpsi/cr/fake_cr.h"
#include "mcpsi/ss/type.h"

namespace test {

class Correlation : public State {
 private:
  std::shared_ptr<Context> ctx_;
  kFp64 key_;

 public:
  static const std::string id;

  // use fake correlation
  std::shared_ptr<FakeCorrelation> fake_cr_ptr_;

  Correlation(std::shared_ptr<Context> ctx) : ctx_(ctx) {
    fake_cr_ptr_ = std::make_shared<FakeCorrelation>(ctx);
  }

  kFp64 GetKey() const { return key_; }
  void SetKey(kFp64 key) {
    key_ = key;
    fake_cr_ptr_->SetKey(key);
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

}  // namespace test
