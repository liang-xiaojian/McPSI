#include "mcpsi/cr/utils/shuffle.h"

#include "yacl/base/dynamic_bitset.h"
#include "yacl/crypto/primitives/ot/gywz_ote.h"
#include "yacl/crypto/utils/rand.h"
#include "yacl/math/gadget.h"

namespace mcpsi::shuffle {

namespace yc = yacl::crypto;
namespace ym = yacl::math;

void ShuffleSend(std::shared_ptr<Connection>& conn,
                 std::shared_ptr<ot::OtAdapter>& ot_ptr,
                 absl::Span<const size_t> perm, absl::Span<internal::PTy> delta,
                 size_t repeat) {
  YACL_ENFORCE(ot_ptr->IsSender() == false);
  const size_t batch_size = perm.size();
  const size_t full_size = delta.size();

  YACL_ENFORCE(batch_size * repeat == full_size);

  for (size_t _ = 0; _ < repeat; ++_) {
    const size_t offset = _ * batch_size;

    const size_t ot_num = ym::Log2Ceil(batch_size);
    const size_t required_ot = batch_size * ot_num;

    std::vector<internal::PTy> a(batch_size, internal::PTy(0));
    std::vector<internal::PTy> b(batch_size, internal::PTy(0));
    std::vector<internal::PTy> opv(batch_size);
    std::vector<uint128_t> punctured_msgs(batch_size);

    std::vector<uint128_t> ot_buff(required_ot);
    yacl::dynamic_bitset<uint128_t> choices(required_ot);

    ot_ptr->recv_rcot(absl::MakeSpan(ot_buff), choices);
    auto ot_store = yc::MakeOtRecvStore(choices, ot_buff);

    for (size_t i = 0; i < batch_size; ++i) {
      auto ot_recv = ot_store.NextSlice(ot_num);
      yc::GywzOtExtRecv(conn, ot_recv, batch_size, perm[i],
                        absl::MakeSpan(punctured_msgs));
      // break correlation
      yc::ParaCrHashInplace_128(absl::MakeSpan(punctured_msgs));
      punctured_msgs[perm[i]] = 0;

      std::transform(punctured_msgs.begin(),
                     punctured_msgs.begin() + batch_size, opv.begin(),
                     [](uint128_t val) { return internal::PTy(val); });
      internal::op::Add(absl::MakeConstSpan(a), absl::MakeConstSpan(opv),
                        absl::MakeSpan(a));
      b[i] = std::reduce(opv.begin(), opv.begin() + batch_size,
                         internal::PTy(0), std::plus<internal::PTy>());
    }
    for (size_t i = 0; i < batch_size; ++i) {
      delta[offset + i] = a[perm[i]] - b[i];
    }
  }
}

void ShuffleRecv(std::shared_ptr<Connection> conn,
                 std::shared_ptr<ot::OtAdapter>& ot_ptr,
                 absl::Span<internal::PTy> a, absl::Span<internal::PTy> b,
                 size_t repeat) {
  YACL_ENFORCE(ot_ptr->IsSender() == true);
  const size_t full_size = a.size();
  const size_t batch_size = full_size / repeat;
  YACL_ENFORCE(full_size == b.size());
  YACL_ENFORCE(batch_size * repeat == full_size);

  for (size_t _ = 0; _ < repeat; ++_) {
    const size_t offset = _ * batch_size;
    const size_t ot_num = ym::Log2Ceil(batch_size);
    const size_t required_ot = batch_size * ot_num;

    auto a_subspan = a.subspan(offset, batch_size);
    auto b_subspan = b.subspan(offset, batch_size);

    std::memset(a_subspan.data(), 0, batch_size * sizeof(internal::PTy));

    std::vector<internal::PTy> opv(batch_size);
    std::vector<uint128_t> all_msgs(batch_size);

    std::vector<uint128_t> ot_buff(required_ot);
    ot_ptr->send_rcot(absl::MakeSpan(ot_buff));
    auto ot_store = yc::MakeCompactOtSendStore(ot_buff, ot_ptr->GetDelta());

    for (size_t i = 0; i < batch_size; ++i) {
      auto ot_send = ot_store.NextSlice(ot_num);
      yc::GywzOtExtSend(conn, ot_send, batch_size, absl::MakeSpan(all_msgs));
      // break correlation
      yc::ParaCrHashInplace_128(absl::MakeSpan(all_msgs));

      std::transform(all_msgs.begin(), all_msgs.begin() + batch_size,
                     opv.begin(),
                     [](uint128_t val) { return internal::PTy(val); });
      internal::op::Sub(absl::MakeConstSpan(a_subspan),
                        absl::MakeConstSpan(opv), absl::MakeSpan(a_subspan));
      b_subspan[i] = std::reduce(opv.begin(), opv.begin() + batch_size,
                                 internal::PTy(0), std::plus<internal::PTy>());
    }
  }
}

}  // namespace mcpsi::shuffle
