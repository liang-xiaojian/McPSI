#include "mcpsi/cr/utils/beaver_helper.h"

#include "mcpsi/utils/vec_op.h"

namespace mcpsi::ot {

namespace {
constexpr size_t kExtFactor = 1;
}

void BeaverHelper::MulPPSender(std::shared_ptr<Connection> conn,
                               absl::Span<internal::PTy> a,
                               absl::Span<internal::PTy> c) {
  const size_t num = a.size();
  YACL_ENFORCE(num == c.size());
  // bits in Public Type
  const size_t PTy_bits = sizeof(internal::PTy) * 8;
  // beaver extend num = num * extend factor
  const size_t ext_num = num * kExtFactor;
  // ot num = num * extend factor * bits of Public Type
  const size_t ot_num = ext_num * PTy_bits;

  auto ext_a = Rand(ext_num);
  auto choices = yacl::dynamic_bitset<uint128_t>(ot_num);
  memcpy(choices.data(), ext_a.data(), ext_num * sizeof(internal::PTy));

  std::vector<uint128_t> ot_recv_msgs(ot_num);
  ot_receiver_->recv_rot(absl::MakeSpan(ot_recv_msgs), choices);

  auto recv_buf = conn->Recv(conn->NextRank(), "Beaver:MulPP");
  auto recv_span = absl::MakeConstSpan(
      reinterpret_cast<internal::PTy *>(recv_buf.data()), ot_num);

  auto ext_c = Zeros(ext_num);
  for (size_t i = 0; i < ext_num; ++i) {
    const size_t offset = i * PTy_bits;
    for (size_t j = 0; j < PTy_bits; ++j) {
      ext_c[i] = ext_c[i] + internal::PTy(ot_recv_msgs[offset + j]);
      if (choices[offset + j]) {
        ext_c[i] = ext_c[i] + internal::PTy(recv_span[offset + j]);
      }
    }
  }
  // sync and generate the coefficient
  auto seed = conn->SyncSeed();
  auto coef = Rand(seed, ext_num);
  Mul(absl::MakeConstSpan(ext_a), absl::MakeConstSpan(coef),
      absl::MakeSpan(ext_a));
  Mul(absl::MakeConstSpan(ext_c), absl::MakeConstSpan(coef),
      absl::MakeSpan(ext_c));

  for (size_t i = 0; i < num; ++i) {
    c[i] = internal::PTy::Zero();
    a[i] = internal::PTy::Zero();
    for (size_t j = 0; j < kExtFactor; ++j) {
      c[i] = c[i] + ext_c[i * kExtFactor + j];
      a[i] = a[i] + ext_a[i * kExtFactor + j];
    }
  }
}

void BeaverHelper::MulPPReceiver(std::shared_ptr<Connection> conn,
                                 absl::Span<internal::PTy> b,
                                 absl::Span<internal::PTy> c) {
  const size_t num = b.size();
  YACL_ENFORCE(num == c.size());
  // bits in Public Type
  const size_t PTy_bits = sizeof(internal::PTy) * 8;
  // beaver extend num = num * extend factor
  const size_t ext_num = num * kExtFactor;
  // ot num = num * extend factor * bits of Public Type
  const size_t ot_num = ext_num * PTy_bits;
  Rand(absl::MakeSpan(b));

  std::vector<std::array<uint128_t, 2>> ot_send_msgs(ot_num);
  ot_sender_->send_rot(absl::MakeSpan(ot_send_msgs));  // rot

  // Convert ROT to additive-COT
  auto send_msgs = std::vector<internal::PTy>(ot_num);
  for (size_t i = 0; i < num; ++i) {
    // ot_block0 - ot_block1 + b
    // c = - block0
    for (size_t j = 0; j < kExtFactor; ++j) {
      const size_t offset = i * kExtFactor * PTy_bits + j * PTy_bits;
      auto bi_k = b[i];
      for (size_t k = 0; k < PTy_bits; ++k) {
        send_msgs[offset + k] = internal::PTy(ot_send_msgs[offset + k][0]) -
                                internal::PTy(ot_send_msgs[offset + k][1]) +
                                bi_k;
        bi_k = bi_k * internal::PTy(2);
      }
    }
  }

  // send
  conn->SendAsync(
      conn->NextRank(),
      yacl::ByteContainerView(send_msgs.data(),
                              send_msgs.size() * sizeof(internal::PTy)),
      "Beaver:MulPP");

  auto ext_c = Zeros(ext_num);
  for (size_t i = 0; i < ext_num; ++i) {
    const size_t offset = i * PTy_bits;
    for (size_t j = 0; j < PTy_bits; ++j) {
      ext_c[i] = ext_c[i] - internal::PTy(ot_send_msgs[offset + j][0]);
    }
  }

  // sync and generate the coefficient
  auto seed = conn->SyncSeed();
  auto coef = Rand(seed, ext_num);
  Mul(absl::MakeConstSpan(ext_c), absl::MakeConstSpan(coef),
      absl::MakeSpan(ext_c));

  for (size_t i = 0; i < num; ++i) {
    c[i] = internal::PTy::Zero();
    for (size_t j = 0; j < kExtFactor; ++j) {
      c[i] = c[i] + ext_c[i * kExtFactor + j];
    }
  }
}

void BeaverHelper::BeaverTriple(std::shared_ptr<Connection> conn,
                                absl::Span<internal::PTy> a,
                                absl::Span<internal::PTy> b,
                                absl::Span<internal::PTy> c) {
  const size_t num = c.size();
  YACL_ENFORCE(num == a.size());
  YACL_ENFORCE(num == b.size());

  auto c0 = Zeros(num);
  auto c1 = Zeros(num);
  if (conn->Rank() == 0) {
    MulPPSender(conn, absl::MakeSpan(a), absl::MakeSpan(c0));
    MulPPReceiver(conn, absl::MakeSpan(b), absl::MakeSpan(c1));
  } else {
    MulPPReceiver(conn, absl::MakeSpan(b), absl::MakeSpan(c0));
    MulPPSender(conn, absl::MakeSpan(a), absl::MakeSpan(c1));
  }
  Add(absl::MakeConstSpan(c0), absl::MakeConstSpan(c1), absl::MakeSpan(c));
}
}  // namespace mcpsi::ot
