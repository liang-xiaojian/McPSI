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
  void BeaverTripleSet(absl::Span<internal::ATy> a, absl::Span<internal::ATy> b,
                       absl::Span<internal::ATy> c) override;
  void BeaverTripleGet(absl::Span<internal::ATy> a, absl::Span<internal::ATy> b,
                       absl::Span<internal::ATy> c) override;

  // entry
  void RandomSet(absl::Span<internal::ATy> out) override;
  void RandomGet(absl::Span<internal::ATy> out) override;
  void RandomAuth(absl::Span<internal::ATy> out) override;

  // entry
  void ShuffleSet(absl::Span<const size_t> perm,
                  absl::Span<internal::PTy> delta, size_t repeat = 1) override;

  void ShuffleGet(absl::Span<internal::PTy> a, absl::Span<internal::PTy> b,
                  size_t repeat = 1) override;
};

}  // namespace mcpsi
