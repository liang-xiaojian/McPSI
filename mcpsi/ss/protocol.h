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

namespace mcpsi {

namespace ym = yacl::math;
namespace yc = yacl::crypto;

using PTy = internal::PTy;
using ATy = internal::ATy;
using GTy = internal::GTy;
using MTy = internal::MTy;
using OP = internal::op;

class Protocol : public State {
 private:
  std::shared_ptr<Context> ctx_;
  // SPDZ key
  PTy key_;

  // DY-PRF
  bool init_prf_{false};
  std::shared_ptr<yc::EcGroup> group_{nullptr};
  GTy g_;  // the generator for PRF
  ATy k_;  // the distributed key for PRF

 public:
  static const std::string id;

  Protocol(std::shared_ptr<Context> ctx) : ctx_(ctx) {
    // SPDZ key setup
    key_ = PTy(yacl::crypto::SecureRandU64());
  }
  // SPDZ key
  PTy GetKey() const { return key_; }

  // DY-PRF
  void SetupPrf() {
    if (init_prf_ == true) {
      return;
    }
    // avoid communication
    group_ = yc::EcGroupFactory::Instance().Create("secp128r2",
                                                   yacl::ArgLib = "openssl");
    g_ = group_->GetGenerator();
    k_ = RandA(1)[0];
    init_prf_ = true;
  }

  std::shared_ptr<yc::EcGroup> GetGroup() const { return group_; }
  GTy GetPrfG() const { return g_; }
  ATy GetPrfK() const { return k_; }
  void RefreshPrfK() { k_ = RandA(1)[0]; }

  // PP evaluation
  std::vector<PTy> Add(absl::Span<const PTy> lhs, absl::Span<const PTy> rhs,
                       bool cache = false);
  std::vector<PTy> Sub(absl::Span<const PTy> lhs, absl::Span<const PTy> rhs,
                       bool cache = false);
  std::vector<PTy> Mul(absl::Span<const PTy> lhs, absl::Span<const PTy> rhs,
                       bool cache = false);
  std::vector<PTy> Div(absl::Span<const PTy> lhs, absl::Span<const PTy> rhs,
                       bool cache = false);

  // AA evalutaion
  std::vector<ATy> Add(absl::Span<const ATy> lhs, absl::Span<const ATy> rhs,
                       bool cache = false);
  std::vector<ATy> Sub(absl::Span<const ATy> lhs, absl::Span<const ATy> rhs,
                       bool cache = false);
  std::vector<ATy> Mul(absl::Span<const ATy> lhs, absl::Span<const ATy> rhs,
                       bool cache = false);
  std::vector<ATy> Div(absl::Span<const ATy> lhs, absl::Span<const ATy> rhs,
                       bool cache = false);

  // AP evalutaion
  std::vector<ATy> Add(absl::Span<const ATy> lhs, absl::Span<const PTy> rhs,
                       bool cache = false);
  std::vector<ATy> Sub(absl::Span<const ATy> lhs, absl::Span<const PTy> rhs,
                       bool cache = false);
  std::vector<ATy> Mul(absl::Span<const ATy> lhs, absl::Span<const PTy> rhs,
                       bool cache = false);
  std::vector<ATy> Div(absl::Span<const ATy> lhs, absl::Span<const PTy> rhs,
                       bool cache = false);

  // PA evalutaion
  std::vector<ATy> Add(absl::Span<const PTy> lhs, absl::Span<const ATy> rhs,
                       bool cache = false);
  std::vector<ATy> Sub(absl::Span<const PTy> lhs, absl::Span<const ATy> rhs,
                       bool cache = false);
  std::vector<ATy> Mul(absl::Span<const PTy> lhs, absl::Span<const ATy> rhs,
                       bool cache = false);
  std::vector<ATy> Div(absl::Span<const PTy> lhs, absl::Span<const ATy> rhs,
                       bool cache = false);

  // convert
  std::vector<PTy> A2P(absl::Span<const ATy> in, bool cache = false);
  std::vector<ATy> P2A(absl::Span<const PTy> in, bool cache = false);
  std::vector<MTy> A2M(absl::Span<const ATy> in, bool cache = false);
  std::vector<GTy> M2G(absl::Span<const MTy> in, bool cache = false);
  std::vector<GTy> A2G(absl::Span<const ATy> in, bool cache = false);

  // others
  std::vector<PTy> Inv(absl::Span<const PTy> in, bool cache = false);
  std::vector<PTy> Neg(absl::Span<const PTy> in, bool cache = false);
  std::vector<ATy> Inv(absl::Span<const ATy> in, bool cache = false);
  std::vector<ATy> Neg(absl::Span<const ATy> in, bool cache = false);

  // special
  std::vector<PTy> ZerosP(size_t num, bool cache = false);
  std::vector<PTy> RandP(size_t num, bool cache = false);
  std::vector<ATy> ZerosA(size_t num, bool cache = false);
  std::vector<ATy> RandA(size_t num, bool cache = false);
  std::vector<ATy> SetA(absl::Span<const PTy> in, bool cache = false);
  std::vector<ATy> GetA(size_t num, bool cache = false);
  // Circuit Operation
  std::vector<ATy> SumA(absl::Span<const ATy> in, bool cache = false);
  // Filter
  std::vector<ATy> FilterA(absl::Span<const ATy> in,
                           absl::Span<const size_t> indexes,
                           bool cache = false);

  // shuffle entry
  std::vector<ATy> ShuffleA(absl::Span<const ATy> in, bool cache = false);
  std::vector<ATy> ShuffleASet(absl::Span<const ATy> in, bool cache = false);
  std::vector<ATy> ShuffleAGet(absl::Span<const ATy> in, bool cache = false);

  // shuffle entry
  std::array<std::vector<ATy>, 2> ShuffleA(absl::Span<const ATy> in0,
                                           absl::Span<const ATy> in1,
                                           bool cache = false);
  std::array<std::vector<ATy>, 2> ShuffleASet(absl::Span<const ATy> in0,
                                              absl::Span<const ATy> in1,
                                              bool cache = false);
  std::array<std::vector<ATy>, 2> ShuffleAGet(absl::Span<const ATy> in0,
                                              absl::Span<const ATy> in1,
                                              bool cache = false);

  std::vector<ATy> ZeroOneA(size_t num, bool cache = false);

  std::vector<ATy> ScalarMulPA(const PTy& scalar, absl::Span<const ATy> in,
                               bool cache = false);

  std::vector<ATy> ScalarMulAP(const ATy& scalar, absl::Span<const PTy> in,
                               bool cache = false);

  std::vector<MTy> ScalarA2M(const ATy& scalar, absl::Span<const ATy> in,
                             bool cache = false);

  std::vector<GTy> ScalarA2G(const ATy& scalar, absl::Span<const ATy> in,
                             bool cache = false);

  // circuit PSI entry
  std::vector<ATy> CPSI(absl::Span<const ATy> set0, absl::Span<const ATy> set1,
                        absl::Span<const ATy> data, bool cache = false);
};

}  // namespace mcpsi
