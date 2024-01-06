#pragma once
#include <memory>
#include <vector>

#include "mcpsi/context/context.h"
#include "mcpsi/context/state.h"
#include "mcpsi/cr/cr.h"
#include "mcpsi/ss/type.h"

namespace mcpsi {

class FakeCorrelation : public Correlation {
 public:
  FakeCorrelation(std::shared_ptr<Context> ctx) : Correlation(ctx) {}

  ~FakeCorrelation() {}

  internal::PTy GetKey() const override { return key_; }

  void SetKey(internal::PTy key) override { key_ = key; }

  void OneTimeSetup() override { ; }

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
};

}  // namespace mcpsi
