#pragma once
#include <memory>
#include <vector>

#include "mcpsi/context/context.h"
#include "mcpsi/context/state.h"
#include "mcpsi/ss/type.h"

namespace mcpsi {

struct BeaverTy {
  std::vector<internal::ATy> a;
  std::vector<internal::ATy> b;
  std::vector<internal::ATy> c;
  BeaverTy(std::vector<internal::ATy>&& aa, std::vector<internal::ATy>&& bb,
           std::vector<internal::ATy>&& cc) {
    a = std::move(aa);
    b = std::move(bb);
    c = std::move(cc);
  }
  BeaverTy(uint32_t n) {
    a.resize(n);
    b.resize(n);
    c.resize(n);
  }
};

struct AuthTy {
  std::vector<internal::ATy> data;
  AuthTy(uint32_t n) { data.resize(n); }
  AuthTy(std::vector<internal::ATy>&& in) { data = std::move(in); }
};

struct ShuffleSTy {
  std::vector<internal::PTy> delta;
  std::vector<size_t> perm;
  ShuffleSTy(uint32_t n, uint32_t repeat = 1) {
    delta.resize(n * repeat);
    perm.resize(n);
  }
  ShuffleSTy(std::vector<internal::PTy>&& _delta, std::vector<size_t>&& _perm) {
    delta = std::move(_delta);
    perm = std::move(_perm);
  }
};

struct ShuffleGTy {
  std::vector<internal::PTy> a;
  std::vector<internal::PTy> b;
  ShuffleGTy(uint32_t n, uint32_t repeat = 1) {
    a.resize(n * repeat);
    b.resize(n * repeat);
  }
  ShuffleGTy(std::vector<internal::PTy>&& _a, std::vector<internal::PTy>&& _b) {
    a = std::move(_a);
    b = std::move(_b);
  }
};

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

  // entry
  virtual void RandomSet(absl::Span<internal::ATy> out) = 0;
  virtual void RandomGet(absl::Span<internal::ATy> out) = 0;
  virtual void RandomAuth(absl::Span<internal::ATy> out) = 0;

  // entry
  virtual void ShuffleSet(absl::Span<const size_t> perm,
                          absl::Span<internal::PTy> delta,
                          size_t repeat = 1) = 0;

  virtual void ShuffleGet(absl::Span<internal::PTy> a,
                          absl::Span<internal::PTy> b, size_t repeat = 1) = 0;

  // interface
  BeaverTy BeaverTriple(size_t num) {
    std::vector<internal::ATy> a(num);
    std::vector<internal::ATy> b(num);
    std::vector<internal::ATy> c(num);
    BeaverTriple(absl::MakeSpan(a), absl::MakeSpan(b), absl::MakeSpan(c));
    return BeaverTy(std::move(a), std::move(b), std::move(c));
  }

  AuthTy RandomSet(size_t num) {
    std::vector<internal::ATy> ret(num);
    RandomSet(absl::MakeSpan(ret));
    return AuthTy(std::move(ret));
  }

  AuthTy RandomGet(size_t num) {
    std::vector<internal::ATy> ret(num);
    RandomGet(absl::MakeSpan(ret));
    return AuthTy(std::move(ret));
  }

  AuthTy RandomAuth(size_t num) {
    std::vector<internal::ATy> ret(num);
    RandomAuth(absl::MakeSpan(ret));
    return AuthTy(std::move(ret));
  }

  ShuffleSTy ShuffleSet(size_t num, size_t repeat = 1) {
    std::vector<internal::PTy> delta(num * repeat);
    std::vector<size_t> perm = GenPerm(num);
    ShuffleSet(absl::MakeSpan(perm), absl::MakeSpan(delta), repeat);
    return ShuffleSTy(std::move(delta), std::move(perm));
  }

  ShuffleGTy ShuffleGet(size_t num, size_t repeat = 1) {
    std::vector<internal::PTy> a(num * repeat);
    std::vector<internal::PTy> b(num * repeat);
    ShuffleGet(absl::MakeSpan(a), absl::MakeSpan(b), repeat);
    return ShuffleGTy(std::move(a), std::move(b));
  }
};

}  // namespace mcpsi
