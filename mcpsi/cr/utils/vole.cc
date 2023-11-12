#include "mcpsi/cr/utils/vole.h"

#include "mcpsi/utils/vec_op.h"
#include "yacl/base/byte_container_view.h"
#include "yacl/crypto/primitives/ot/gywz_ote.h"
#include "yacl/crypto/tools/random_permutation.h"

namespace mcpsi::vole {

void MpVoleSend(const std::shared_ptr<Connection>& conn,
                const yc::OtSendStore& send_ot, const MpParam& param,
                absl::Span<internal::PTy> w, absl::Span<internal::PTy> output) {
  YACL_ENFORCE(output.size() >= param.mp_vole_size_);
  YACL_ENFORCE(w.size() >= param.noise_num_);
  YACL_ENFORCE(send_ot.Size() >= param.require_ot_num_);

  const auto& batch_num = param.noise_num_;
  const auto& batch_size = param.sp_vole_size_;
  const auto& last_batch_size = param.last_sp_vole_size_;
  const auto batch_length = ym::Log2Ceil(batch_size);
  const auto last_batch_length = ym::Log2Ceil(last_batch_size);

  std::vector<uint128_t> ote_buffer(param.mp_vole_size_);
  auto ote_span = absl::MakeSpan(ote_buffer);

  for (size_t i = 0; i < batch_num; ++i) {
    auto this_size = (i == batch_num - 1) ? last_batch_size : batch_size;
    auto this_length = (i == batch_num - 1) ? last_batch_length : batch_length;

    auto this_span = ote_span.subspan(i * batch_size, this_size);
    auto ot_slice =
        send_ot.Slice(i * batch_length, i * batch_length + this_length);

    yc::GywzOtExtSend(conn, ot_slice, this_size, this_span);
  }

  yc::ParaCrHashInplace_128(ote_span);

  auto send_msgs = std::vector<internal::PTy>(w.data(), w.data() + batch_num);

  for (size_t i = 0; i < batch_num; ++i) {
    auto this_size = (i == batch_num - 1) ? last_batch_size : batch_size;
    auto this_span = ote_span.subspan(i * batch_size, this_size);
    auto output_span = output.subspan(i * batch_size, this_size);

    for (size_t j = 0; j < this_size; ++j) {
      output_span[j] = internal::PTy(0) - internal::PTy(this_span[j]);
      send_msgs[i] = send_msgs[i] - output_span[j];
    }
  }

  conn->SendAsync(
      conn->NextRank(),
      yacl::ByteContainerView(send_msgs.data(),
                              send_msgs.size() * sizeof(internal::PTy)),
      "MpVole");
}

void MpVoleRecv(const std::shared_ptr<Connection>& conn,
                const yc::OtRecvStore& recv_ot, const MpParam& param,
                absl::Span<internal::PTy> v, absl::Span<internal::PTy> output) {
  YACL_ENFORCE(output.size() >= param.mp_vole_size_);
  YACL_ENFORCE(v.size() >= param.noise_num_);
  YACL_ENFORCE(recv_ot.Size() >= param.require_ot_num_);

  const auto& batch_num = param.noise_num_;
  const auto& batch_size = param.sp_vole_size_;
  const auto& last_batch_size = param.last_sp_vole_size_;
  const auto& indexes = param.indexes_;
  const auto batch_length = ym::Log2Ceil(batch_size);
  const auto last_batch_length = ym::Log2Ceil(last_batch_size);

  std::vector<uint128_t> ote_buffer(param.mp_vole_size_);
  auto ote_span = absl::MakeSpan(ote_buffer);

  for (size_t i = 0; i < batch_num; ++i) {
    auto this_size = (i == batch_num - 1) ? last_batch_size : batch_size;
    auto this_length = (i == batch_num - 1) ? last_batch_length : batch_length;

    auto this_span = ote_span.subspan(i * batch_size, this_size);
    auto ot_slice =
        recv_ot.Slice(i * batch_length, i * batch_length + this_length);

    yc::GywzOtExtRecv(conn, ot_slice, this_size, indexes[i], this_span);
  }

  yc::ParaCrHashInplace_128(ote_span);

  auto recv_buff = conn->Recv(conn->NextRank(), "MpVole");

  auto recv_msgs = absl::MakeSpan(
      reinterpret_cast<internal::PTy*>(recv_buff.data()), batch_num);

  for (size_t i = 0; i < batch_num; ++i) {
    auto this_size = (i == batch_num - 1) ? last_batch_size : batch_size;
    auto this_span = ote_span.subspan(i * batch_size, this_size);
    auto output_span = output.subspan(i * batch_size, this_size);

    this_span[indexes[i]] = 0;

    for (size_t j = 0; j < this_size; ++j) {
      output_span[j] = internal::PTy(this_span[j]);
      recv_msgs[i] = recv_msgs[i] - output_span[j];
    }

    output_span[indexes[i]] = recv_msgs[i] + v[i];
  }
}

}  // namespace mcpsi::vole
