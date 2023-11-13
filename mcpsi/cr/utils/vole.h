#pragma once
#include "mcpsi/context/state.h"
#include "mcpsi/ss/type.h"
#include "mcpsi/utils/field.h"
#include "yacl/crypto/primitives/ot/ot_store.h"
#include "yacl/math/gadget.h"

namespace mcpsi::vole {

namespace ym = yacl::math;
namespace yc = yacl::crypto;

// [Warning] MpVole && Wolverine Vole are semi-honest version
// TODO: change them into malicious version

// Multi-point Vole Parameters
struct MpParam {
  size_t sp_vole_size_;
  size_t last_sp_vole_size_;

  size_t mp_vole_size_;
  size_t noise_num_;
  size_t require_ot_num_;
  std::vector<size_t> indexes_;

  MpParam(size_t mp_vole_size, size_t noise_num) {
    YACL_ENFORCE(mp_vole_size >= 2 * noise_num);
    mp_vole_size_ = mp_vole_size;
    noise_num_ = noise_num;
    //
    sp_vole_size_ = mp_vole_size_ / noise_num_;
    last_sp_vole_size_ = mp_vole_size_ - sp_vole_size_ * (noise_num_ - 1);
    require_ot_num_ = ym::Log2Ceil(sp_vole_size_) +
                      ym::Log2Ceil(last_sp_vole_size_) * (noise_num_ - 1);
  };

  // [Warning] indexes is not strictly uniform
  void GenIndexes() {
    indexes_ = yc::RandVec<size_t>(noise_num_);
    for (size_t i = 0; i < noise_num_ - 1; ++i) {
      indexes_[i] %= sp_vole_size_;
    }
    indexes_[noise_num_ - 1] %= last_sp_vole_size_;
  }

  // [Warning] it won't check whether the range of indexes is correct
  void SetIndexes(absl::Span<const size_t> indexes) {
    YACL_ENFORCE(indexes.size() >= noise_num_);
    for (size_t i = 0; i < noise_num_ - 1; ++i) {
      indexes_[i] = indexes[i] % sp_vole_size_;
    }
    indexes_[noise_num_ - 1] = indexes[noise_num_ - 1] % last_sp_vole_size_;
  }
};

// Multi-point Vole
void MpVoleSend(const std::shared_ptr<Connection>& conn,
                const yc::OtSendStore& send_ot, const MpParam& param,
                absl::Span<internal::PTy> w, absl::Span<internal::PTy> output);

void MpVoleRecv(const std::shared_ptr<Connection>& conn,
                const yc::OtRecvStore& recv_ot, const MpParam& param,
                absl::Span<internal::PTy> v, absl::Span<internal::PTy> output);

struct LpnParam {
  size_t n_ = 10485760;
  size_t k_ = 452000;
  size_t t_ = 1280;

  // MpVole
  MpParam mp_param_{10485760, 1280};

  LpnParam() : LpnParam(10485760, 452000, 1280) {}

  LpnParam(size_t n, size_t k, size_t t) : n_(n), k_(k), t_(t) {
    mp_param_ = MpParam(n_, t_);
  }

  void GenIndexes() { mp_param_.GenIndexes(); }

  void SetIndexes(absl::Span<const size_t> indexes) {
    mp_param_.SetIndexes(indexes);
  }

  static LpnParam GetDefault() { return LpnParam(10485760, 452000, 1280); }

  static LpnParam GetPreDefault() { return LpnParam(470016, 32768, 918); }
};

// Wolverine Vole
// the following equation would hold:
// > pre_c = pre_a * delta + pre_b
// > c     =     a * delta +     b
void WolverineVoleSend(const std::shared_ptr<Connection>& conn,
                       const yc::OtSendStore& send_ot, const LpnParam& param,
                       absl::Span<internal::PTy> pre_c,
                       absl::Span<internal::PTy> c);

void WolverineVoleRecv(const std::shared_ptr<Connection>& conn,
                       const yc::OtRecvStore& recv_ot, const LpnParam& param,
                       absl::Span<internal::PTy> pre_a,
                       absl::Span<internal::PTy> pre_b,
                       absl::Span<internal::PTy> a,
                       absl::Span<internal::PTy> b);

}  // namespace mcpsi::vole
