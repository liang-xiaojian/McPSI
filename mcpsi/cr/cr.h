#pragma once
#include <memory>
#include <vector>

#include "mcpsi/context/context.h"
#include "mcpsi/context/state.h"
#include "mcpsi/ss/type.h"

namespace mcpsi {

class Correlation : public State {
 protected:
  std::shared_ptr<Context> ctx_;
  internal::PTy key_;

 public:
  static const std::string id;

  Correlation(std::shared_ptr<Context> ctx) : ctx_(ctx) {}

  virtual ~Correlation() {}

  virtual internal::PTy GetKey() const { return key_; }

  virtual void SetKey(internal::PTy key) { key_ = key; }

  virtual void OneTimeSetup() = 0;

  // entry
  virtual void BeaverTriple(absl::Span<internal::ATy> a,
                            absl::Span<internal::ATy> b,
                            absl::Span<internal::ATy> c) = 0;

  virtual std::array<std::vector<internal::ATy>, 3> BeaverTriple(
      size_t num) = 0;

  virtual void AuthSet(absl::Span<const internal::PTy> in,
                       absl::Span<internal::ATy> out) = 0;
  virtual void AuthGet(absl::Span<internal::ATy> out) = 0;

  // entry
  virtual void RandomSet(absl::Span<internal::ATy> out) = 0;
  virtual void RandomGet(absl::Span<internal::ATy> out) = 0;
  virtual void RandomAuth(absl::Span<internal::ATy> out) = 0;

  virtual std::vector<internal::ATy> RandomSet(size_t num) = 0;
  virtual std::vector<internal::ATy> RandomGet(size_t num) = 0;
  virtual std::vector<internal::ATy> RandomAuth(size_t num) = 0;

  // entry
  virtual void ShuffleSet(absl::Span<const size_t> perm,
                          absl::Span<internal::PTy> delta,
                          size_t repeat = 1) = 0;

  virtual void ShuffleGet(absl::Span<internal::PTy> a,
                          absl::Span<internal::PTy> b, size_t repeat = 1) = 0;

  virtual std::vector<internal::PTy> ShuffleSet(absl::Span<const size_t> perm,
                                                size_t repeat = 1) = 0;

  virtual std::pair<std::vector<internal::PTy>, std::vector<internal::PTy>>
  ShuffleGet(size_t num, size_t repeat = 1) = 0;
};

}  // namespace mcpsi
