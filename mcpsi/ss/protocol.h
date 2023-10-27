#pragma once
#include <vector>

#include "mcpsi/context/context.h"
#include "mcpsi/context/state.h"
#include "mcpsi/ss/ashare.h"
#include "mcpsi/ss/gshare.h"
#include "mcpsi/ss/public.h"
#include "mcpsi/ss/type.h"
#include "mcpsi/utils/config.h"
#include "yacl/crypto/utils/rand.h"

namespace test {

namespace ym = yacl::math;

using PTy = internal::PTy;
using ATy = internal::ATy;
using GTy = internal::GTy;
using MTy = internal::MTy;

class Protocol : public State {
 private:
  std::shared_ptr<Context> ctx_;
  // SPDZ key
  PTy key_;

  // DY-PRF
  bool init_prf_{false};
  GTy mod_;  // the module for PRF
  GTy g_;    // the generator for PRF
  ATy k_;    // the distributed key for PRF

 public:
  static const std::string id;

  Protocol(std::shared_ptr<Context> ctx) : ctx_(ctx) {
    // SPDZ key setup
    key_ = PTy(yacl::crypto::RandU64(true));
  }
  // SPDZ key
  PTy GetKey() const { return key_; }

  // DY-PRF
  void SetupPrf() {
    if (init_prf_ == true) {
      return;
    }
    // DY-PRF setup
    mod_ = ym::MPInt(Prime64 * 2 + 1);
    YACL_ENFORCE(mod_.IsPrime());
    // avoid communication
    // TEST ME: whether is secure
    uint64_t r64 = 0;
    ctx_->GetState<Prg>()->Fill(absl::MakeSpan(&r64, 1));
    g_ = ym::MPInt(r64).PowMod(ym::MPInt(2), mod_);
    k_ = RandA(1)[0];
    init_prf_ = true;
  }

  GTy GetPrfMod() const { return mod_; }
  GTy GetPrfG() const { return g_; }
  ATy GetPrfK() const { return k_; }
  void RefreshPrfK() { k_ = RandA(1)[0]; }

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
  std::vector<MTy> A2M(absl::Span<const ATy> in);
  std::vector<GTy> M2G(absl::Span<const MTy> in);
  std::vector<GTy> A2G(absl::Span<const ATy> in);

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
  std::vector<ATy> SetA(absl::Span<const PTy> in);
  std::vector<ATy> GetA(size_t num);
  // Circuit Operation
  std::vector<ATy> SumA(absl::Span<const ATy> in);
  // Filter
  std::vector<ATy> FilterA(absl::Span<const ATy> in,
                           absl::Span<const size_t> indexes);

  std::vector<ATy> ShuffleA(absl::Span<const ATy> in);
  // shuffle entry
  std::vector<ATy> ShuffleA(absl::Span<const ATy> in,
                            absl::Span<const size_t> perm);
  std::vector<ATy> ShuffleASet(absl::Span<const ATy> in,
                               absl::Span<const size_t> perm);
  std::vector<ATy> ShuffleAGet(absl::Span<const ATy> in);
};

}  // namespace test
