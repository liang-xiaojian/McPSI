#pragma once
#include <vector>

#include "test/context/context.h"
#include "test/context/state.h"
#include "test/ss/ashare.h"
#include "test/ss/public.h"
#include "test/ss/type.h"
#include "test/utils/config.h"
#include "yacl/crypto/utils/rand.h"

namespace test {

using PTy = internal::PTy;
using ATy = internal::ATy;
// using MTy = internal::MTy;

// namespace ym = yacl::math;

class Protocol : public State {
 private:
  std::shared_ptr<Context> ctx_;
  // SPDZ key
  PTy key_;
  // DY-PRF
  //   MTy mod_;
  //   MTy g_;
  //   MTy k_;

 public:
  static const std::string id;

  Protocol(std::shared_ptr<Context> ctx) : ctx_(ctx) {
    // key_ = PTy(yacl::crypto::RandU64(true));
    // mod_ = ym::MPInt(Prime64 * 2 + 1);
    // YACL_ENFORCE(mod_.IsPrime());
    // uint128_t r128 = ctx_->SyncSeed();
    // auto [high, low] = yacl::DecomposeUInt128(r128);
    // k_ = ym::MPInt(high).Mod(ym::MPInt(Prime64));
    // g_ = ym::MPInt(low).PowMod(ym::MPInt(2), mod_);
  }

  PTy GetKey() const { return key_; }

  //   MTy GetPrfMod() const { return mod_; }

  //   MTy GetPrfG() const { return g_; }

  //   MTy GetPrfK() const { return k_; }

  // void RefreshPrfK() {
  //   uint128_t r128 = ctx_->SyncSeed();
  //   k_ = ym::MPInt(r128).Mod(ym::MPInt(Prime64));
  // }

  // PP evaluation
  std::vector<PTy> Add(absl::Span<const PTy> lhs, absl::Span<const PTy> rhs);
  std::vector<PTy> Sub(absl::Span<const PTy> lhs, absl::Span<const PTy> rhs);
  std::vector<PTy> Mul(absl::Span<const PTy> lhs, absl::Span<const PTy> rhs);
  std::vector<PTy> Div(absl::Span<const PTy> lhs, absl::Span<const PTy> rhs);

  // AA evalutaion
  std::vector<ATy> Add(absl::Span<const ATy> lhs, absl::Span<const ATy> rhs);
  std::vector<ATy> Sub(absl::Span<const ATy> lhs, absl::Span<const ATy> rhs);
  std::vector<ATy> Mul(absl::Span<const ATy> lhs, absl::Span<const ATy> rhs);
  std::vector<ATy> Div(absl::Span<const ATy> lhs, absl::Span<const ATy> rhs);

  // AP evalutaion
  std::vector<ATy> Add(absl::Span<const ATy> lhs, absl::Span<const PTy> rhs);
  std::vector<ATy> Sub(absl::Span<const ATy> lhs, absl::Span<const PTy> rhs);
  std::vector<ATy> Mul(absl::Span<const ATy> lhs, absl::Span<const PTy> rhs);
  std::vector<ATy> Div(absl::Span<const ATy> lhs, absl::Span<const PTy> rhs);

  // PA evalutaion
  std::vector<ATy> Add(absl::Span<const PTy> lhs, absl::Span<const ATy> rhs);
  std::vector<ATy> Sub(absl::Span<const PTy> lhs, absl::Span<const ATy> rhs);
  std::vector<ATy> Mul(absl::Span<const PTy> lhs, absl::Span<const ATy> rhs);
  std::vector<ATy> Div(absl::Span<const PTy> lhs, absl::Span<const ATy> rhs);

  // convert
  std::vector<PTy> A2P(absl::Span<const ATy> in);
  std::vector<ATy> P2A(absl::Span<const PTy> in);

  // others
  std::vector<PTy> Inv(absl::Span<const PTy> in);
  std::vector<PTy> Neg(absl::Span<const PTy> in);
  std::vector<ATy> Inv(absl::Span<const ATy> in);
  std::vector<ATy> Neg(absl::Span<const ATy> in);

  // special
  std::vector<PTy> ZerosP(size_t num);
  std::vector<PTy> RandP(size_t num);
  std::vector<ATy> ZerosA(size_t num);
  std::vector<ATy> RandA(size_t num);
  std::vector<ATy> ShuffleA(absl::Span<const ATy> in);

  // shuffle entry
  std::vector<ATy> ShuffleA(absl::Span<const ATy> in,
                            absl::Span<const size_t> perm);
  std::vector<ATy> ShuffleASet(absl::Span<const ATy> in,
                               absl::Span<const size_t> perm);
  std::vector<ATy> ShuffleAGet(absl::Span<const ATy> in);
};

}  // namespace test
