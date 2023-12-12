#pragma once
#include "mcpsi/context/state.h"
#include "mcpsi/cr/utils/mpfss.h"
#include "mcpsi/ss/type.h"
#include "mcpsi/utils/field.h"
#include "yacl/crypto/primitives/ot/ot_store.h"
#include "yacl/math/gadget.h"

namespace mcpsi::vole {

namespace ym = yacl::math;
namespace yc = yacl::crypto;

struct LpnParam {
  size_t n_ = 10485760;
  size_t k_ = 452000;
  size_t t_ = 1280;

  LpnParam() : LpnParam(10485760, 452000, 1280) {}
  LpnParam(size_t n, size_t k, size_t t) : n_(n), k_(k), t_(t) {}

  static LpnParam GetDefault() { return LpnParam(10485760, 452000, 1280); }
  static LpnParam GetPreDefault() { return LpnParam(470016, 32768, 918); }
};

struct VoleParam {
  LpnParam lpn_param_ = LpnParam::GetDefault();
  MpParam mp_param_{10485760, 1280};

  bool is_mal_ = false;
  size_t extra_ = 0;

  VoleParam() : VoleParam(LpnParam::GetDefault()) {}

  VoleParam(LpnParam lpn_param, bool mal = false) {
    lpn_param_ = lpn_param;
    mp_param_ = MpParam(lpn_param.n_, lpn_param.t_);

    is_mal_ = mal;
    extra_ = is_mal_ == true ? 1 : 0;
  }

  void GenIndexes() { mp_param_.GenIndexes(); }

  void SetIndexes(absl::Span<const size_t> indexes) {
    mp_param_.SetIndexes(indexes);
  }
};

// Wolverine Vole
// the following equation would hold:
// > pre_c = pre_a * delta + pre_b
// > c     =     a * delta +     b
// > Sender holds c && delta
// > Receiver holds a && b
void WolverineVoleSend(const std::shared_ptr<Connection>& conn,
                       const yc::OtSendStore& send_ot, const VoleParam& param,
                       absl::Span<internal::PTy> pre_c,
                       absl::Span<internal::PTy> c);

void WolverineVoleRecv(const std::shared_ptr<Connection>& conn,
                       const yc::OtRecvStore& recv_ot, const VoleParam& param,
                       absl::Span<internal::PTy> pre_a,
                       absl::Span<internal::PTy> pre_b,
                       absl::Span<internal::PTy> a,
                       absl::Span<internal::PTy> b);

}  // namespace mcpsi::vole
