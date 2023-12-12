#include "mcpsi/cr/utils/vole.h"

#include "mcpsi/cr/utils/linear_code.h"
#include "mcpsi/utils/vec_op.h"
#include "yacl/base/byte_container_view.h"

namespace mcpsi::vole {

void WolverineVoleSend(const std::shared_ptr<Connection>& conn,
                       const yc::OtSendStore& send_ot, const VoleParam& param,
                       absl::Span<internal::PTy> pre_c,
                       absl::Span<internal::PTy> c) {
  auto& lpn_param = param.lpn_param_;
  auto& mp_param = param.mp_param_;

  YACL_ENFORCE(pre_c.size() >= lpn_param.k_);
  YACL_ENFORCE(c.size() >= lpn_param.n_);
  YACL_ENFORCE(send_ot.Size() >= mp_param.require_ot_num_);

  MpVoleSend(conn, send_ot, mp_param, pre_c, c);

  auto seed = conn->SyncSeed();
  auto llc = code::LocalLinearCode<10>(seed, lpn_param.n_, lpn_param.k_);
  llc.Encode(pre_c.subspan(0, lpn_param.k_), c.subspan(0, lpn_param.n_));
}

void WolverineVoleRecv(const std::shared_ptr<Connection>& conn,
                       const yc::OtRecvStore& recv_ot, const VoleParam& param,
                       absl::Span<internal::PTy> pre_a,
                       absl::Span<internal::PTy> pre_b,
                       absl::Span<internal::PTy> a,
                       absl::Span<internal::PTy> b) {
  auto& lpn_param = param.lpn_param_;
  auto& mp_param = param.mp_param_;

  YACL_ENFORCE(pre_a.size() >= lpn_param.k_);
  YACL_ENFORCE(pre_b.size() >= lpn_param.k_);
  YACL_ENFORCE(a.size() >= lpn_param.n_);
  YACL_ENFORCE(b.size() >= lpn_param.n_);
  YACL_ENFORCE(recv_ot.Size() >= mp_param.require_ot_num_);

  std::for_each(a.begin(), a.end(), [](internal::PTy& e) { e = 0; });
  for (size_t i = 0; i < mp_param.noise_num_; ++i) {
    a[i * mp_param.sp_vole_size_ + mp_param.indexes_[i]] = pre_a[i];
  }
  MpVoleRecv(conn, recv_ot, mp_param, pre_b, b);

  auto seed = conn->SyncSeed();
  auto llc = code::LocalLinearCode<10>(seed, lpn_param.n_, lpn_param.k_);
  llc.Encode2(pre_a.subspan(0, lpn_param.k_), a.subspan(0, lpn_param.n_),
              pre_b.subspan(0, lpn_param.k_), b.subspan(0, lpn_param.n_));
}

}  // namespace mcpsi::vole
