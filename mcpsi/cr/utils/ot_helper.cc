#include "mcpsi/cr/utils/ot_helper.h"

#include "mcpsi/utils/vec_op.h"
#include "yacl/crypto/primitives/ot/gywz_ote.h"
#include "yacl/crypto/tools/random_permutation.h"
#include "yacl/math/gadget.h"

namespace mcpsi::ot {

namespace yc = yacl::crypto;
namespace ym = yacl::math;

namespace {
// https://eprint.iacr.org/2016/505.pdf
// MASCOT
constexpr size_t kExtFactor = 3;
}  // namespace

// ---------------------
//  Basic Beaver Triple
// ---------------------

void OtHelper::MulPPSend(std::shared_ptr<Connection> conn,
                         absl::Span<internal::PTy> b,
                         absl::Span<internal::PTy> c) {
  const size_t num = b.size();
  YACL_ENFORCE(num == c.size());
  // bits in Public Type
  const size_t PTy_bits = sizeof(internal::PTy) * 8;
  // ot num = num * bits of Public Type
  const size_t ot_num = num * PTy_bits;
  internal::op::Rand(absl::MakeSpan(b));

  std::vector<std::array<uint128_t, 2>> ot_send_msgs(ot_num);
  ot_sender_->send_rot(absl::MakeSpan(ot_send_msgs));  // rot

  // Convert ROT to additive-COT
  auto send_msgs = std::vector<internal::PTy>(ot_num);
  for (size_t i = 0; i < num; ++i) {
    // ot_block0 - ot_block1 + b
    // c = - block0
    const size_t offset = i * PTy_bits;
    auto bi_k = b[i];
    for (size_t k = 0; k < PTy_bits; ++k) {
      send_msgs[offset + k] = internal::PTy(ot_send_msgs[offset + k][0]) -
                              internal::PTy(ot_send_msgs[offset + k][1]) + bi_k;
      bi_k = bi_k * internal::PTy(2);
    }
  }

  // send
  conn->SendAsync(
      conn->NextRank(),
      yacl::ByteContainerView(send_msgs.data(),
                              send_msgs.size() * sizeof(internal::PTy)),
      "Beaver:MulPP");

  for (size_t i = 0; i < num; ++i) {
    c[i] = internal::PTy::Zero();
    const size_t offset = i * PTy_bits;
    for (size_t j = 0; j < PTy_bits; ++j) {
      c[i] = c[i] - internal::PTy(ot_send_msgs[offset + j][0]);
    }
  }
}

void OtHelper::MulPPRecv(std::shared_ptr<Connection> conn,
                         absl::Span<internal::PTy> a,
                         absl::Span<internal::PTy> c) {
  const size_t num = a.size();
  YACL_ENFORCE(num == c.size());
  // bits in Public Type
  const size_t PTy_bits = sizeof(internal::PTy) * 8;
  // ot num = num * bits of Public Type
  const size_t ot_num = num * PTy_bits;

  internal::op::Rand(a);  // rand a
  auto choices = yacl::dynamic_bitset<uint128_t>(ot_num);
  memcpy(choices.data(), a.data(), num * sizeof(internal::PTy));

  std::vector<uint128_t> ot_recv_msgs(ot_num);
  ot_receiver_->recv_rot(absl::MakeSpan(ot_recv_msgs), choices);

  auto recv_buf = conn->Recv(conn->NextRank(), "Beaver:MulPP");
  auto recv_span = absl::MakeConstSpan(
      reinterpret_cast<internal::PTy *>(recv_buf.data()), ot_num);

  for (size_t i = 0; i < num; ++i) {
    c[i] = internal::PTy::Zero();
    const size_t offset = i * PTy_bits;
    for (size_t j = 0; j < PTy_bits; ++j) {
      c[i] = c[i] + internal::PTy(ot_recv_msgs[offset + j]);
      if (choices[offset + j]) {
        c[i] = c[i] + internal::PTy(recv_span[offset + j]);
      }
    }
  }
}

void OtHelper::BeaverTriple(std::shared_ptr<Connection> conn,
                            absl::Span<internal::PTy> a,
                            absl::Span<internal::PTy> b,
                            absl::Span<internal::PTy> c) {
  const size_t num = c.size();
  YACL_ENFORCE(num == a.size());
  YACL_ENFORCE(num == b.size());

  auto c0 = internal::op::Zeros(num);
  auto c1 = internal::op::Zeros(num);
  if (conn->Rank() == 0) {
    MulPPSend(conn, absl::MakeSpan(b), absl::MakeSpan(c0));
    MulPPRecv(conn, absl::MakeSpan(a), absl::MakeSpan(c1));
  } else {
    MulPPRecv(conn, absl::MakeSpan(a), absl::MakeSpan(c0));
    MulPPSend(conn, absl::MakeSpan(b), absl::MakeSpan(c1));
  }
  internal::op::Add(absl::MakeConstSpan(c0), absl::MakeConstSpan(c1),
                    absl::MakeSpan(c));
  // a*b
  internal::op::Mul(absl::MakeConstSpan(a), absl::MakeConstSpan(b),
                    absl::MakeSpan(c0));
  internal::op::Add(absl::MakeConstSpan(c), absl::MakeConstSpan(c0),
                    absl::MakeSpan(c));
}

// ---------------------
//  Extend Beaver Triple
// ---------------------

void OtHelper::MulPPExtendSend(std::shared_ptr<Connection> conn,
                               absl::Span<internal::PTy> b,
                               absl::Span<internal::PTy> c,
                               absl::Span<internal::PTy> C) {
  const size_t num = b.size();
  YACL_ENFORCE(num == c.size());
  // bits in Public Type
  const size_t PTy_bits = sizeof(internal::PTy) * 8;
  // beaver extend num = num * extend factor
  const size_t ext_num = num * kExtFactor;
  // ot num = num * extend factor * bits of Public Type
  const size_t ot_num = ext_num * PTy_bits;
  internal::op::Rand(absl::MakeSpan(b));

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

  auto ext_c = internal::op::Zeros(ext_num);
  for (size_t i = 0; i < ext_num; ++i) {
    const size_t offset = i * PTy_bits;
    for (size_t j = 0; j < PTy_bits; ++j) {
      ext_c[i] = ext_c[i] - internal::PTy(ot_send_msgs[offset + j][0]);
    }
  }

  // sync and generate the coefficient
  auto seed = conn->SyncSeed();
  auto coef = internal::op::Rand(seed, ext_num * 2);

  auto coef_span = absl::MakeConstSpan(coef);
  //
  auto cc = internal::op::Mul(absl::MakeConstSpan(ext_c),
                              coef_span.subspan(0, ext_num));
  //
  auto CC = internal::op::Mul(absl::MakeConstSpan(ext_c),
                              coef_span.subspan(ext_num, ext_num));

  for (size_t i = 0; i < num; ++i) {
    c[i] = internal::PTy::Zero();
    C[i] = internal::PTy::Zero();
    for (size_t j = 0; j < kExtFactor; ++j) {
      c[i] = c[i] + cc[i * kExtFactor + j];
      C[i] = C[i] + CC[i * kExtFactor + j];
    }
  }
}

void OtHelper::MulPPExtendRecv(std::shared_ptr<Connection> conn,
                               absl::Span<internal::PTy> a,
                               absl::Span<internal::PTy> c,
                               absl::Span<internal::PTy> A,
                               absl::Span<internal::PTy> C) {
  const size_t num = a.size();
  YACL_ENFORCE(num == c.size());
  // bits in Public Type
  const size_t PTy_bits = sizeof(internal::PTy) * 8;
  // beaver extend num = num * extend factor
  const size_t ext_num = num * kExtFactor;
  // ot num = num * extend factor * bits of Public Type
  const size_t ot_num = ext_num * PTy_bits;

  auto ext_a = internal::op::Rand(ext_num);
  auto choices = yacl::dynamic_bitset<uint128_t>(ot_num);
  memcpy(choices.data(), ext_a.data(), ext_num * sizeof(internal::PTy));

  std::vector<uint128_t> ot_recv_msgs(ot_num);
  ot_receiver_->recv_rot(absl::MakeSpan(ot_recv_msgs), choices);

  auto recv_buf = conn->Recv(conn->NextRank(), "Beaver:MulPP");
  auto recv_span = absl::MakeConstSpan(
      reinterpret_cast<internal::PTy *>(recv_buf.data()), ot_num);

  auto ext_c = internal::op::Zeros(ext_num);
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
  auto coef = internal::op::Rand(seed, ext_num * 2);
  auto coef_span = absl::MakeConstSpan(coef);

  auto aa = internal::op::Mul(absl::MakeConstSpan(ext_a),
                              coef_span.subspan(0, ext_num));
  auto cc = internal::op::Mul(absl::MakeConstSpan(ext_c),
                              coef_span.subspan(0, ext_num));

  auto AA = internal::op::Mul(absl::MakeConstSpan(ext_a),
                              coef_span.subspan(ext_num, ext_num));
  auto CC = internal::op::Mul(absl::MakeConstSpan(ext_c),
                              coef_span.subspan(ext_num, ext_num));

  for (size_t i = 0; i < num; ++i) {
    c[i] = internal::PTy::Zero();
    a[i] = internal::PTy::Zero();

    C[i] = internal::PTy::Zero();
    A[i] = internal::PTy::Zero();
    for (size_t j = 0; j < kExtFactor; ++j) {
      c[i] = c[i] + cc[i * kExtFactor + j];
      a[i] = a[i] + aa[i * kExtFactor + j];

      C[i] = C[i] + CC[i * kExtFactor + j];
      A[i] = A[i] + AA[i * kExtFactor + j];
    }
  }
}

void OtHelper::BeaverTripleExtend(std::shared_ptr<Connection> conn,
                                  absl::Span<internal::PTy> a,
                                  absl::Span<internal::PTy> b,
                                  absl::Span<internal::PTy> c,
                                  absl::Span<internal::PTy> A,
                                  absl::Span<internal::PTy> C) {
  const size_t num = c.size();
  YACL_ENFORCE(num == a.size());
  YACL_ENFORCE(num == b.size());
  YACL_ENFORCE(num == A.size());
  YACL_ENFORCE(num == C.size());

  auto c0 = internal::op::Zeros(num);
  auto c1 = internal::op::Zeros(num);

  auto C0 = internal::op::Zeros(num);
  auto C1 = internal::op::Zeros(num);
  if (conn->Rank() == 0) {
    MulPPExtendSend(conn, absl::MakeSpan(b), absl::MakeSpan(c0),
                    absl::MakeSpan(C0));
    MulPPExtendRecv(conn, absl::MakeSpan(a), absl::MakeSpan(c1),
                    absl::MakeSpan(A), absl::MakeSpan(C1));
  } else {
    MulPPExtendRecv(conn, absl::MakeSpan(a), absl::MakeSpan(c1),
                    absl::MakeSpan(A), absl::MakeSpan(C1));
    MulPPExtendSend(conn, absl::MakeSpan(b), absl::MakeSpan(c0),
                    absl::MakeSpan(C0));
  }
  internal::op::Add(absl::MakeConstSpan(c0), absl::MakeConstSpan(c1),
                    absl::MakeSpan(c));
  // a*b
  internal::op::Mul(absl::MakeConstSpan(a), absl::MakeConstSpan(b),
                    absl::MakeSpan(c0));
  internal::op::Add(absl::MakeConstSpan(c), absl::MakeConstSpan(c0),
                    absl::MakeSpan(c));

  internal::op::Add(absl::MakeConstSpan(C0), absl::MakeConstSpan(C1),
                    absl::MakeSpan(C));
  // A*b
  internal::op::Mul(absl::MakeConstSpan(A), absl::MakeConstSpan(b),
                    absl::MakeSpan(C0));
  internal::op::Add(absl::MakeConstSpan(C), absl::MakeConstSpan(C0),
                    absl::MakeSpan(C));
}
// ----------------
//  Base Vole
// ----------------
void OtHelper::BaseVoleSend(std::shared_ptr<Connection> conn,
                            internal::PTy delta, absl::Span<internal::PTy> c) {
  const size_t num = c.size();
  const size_t ext_num = num + 1;
  size_t PTy_bits = sizeof(internal::PTy) * 8;
  size_t ot_num = ext_num * PTy_bits;

  // initalize delta , delta * 2 , ...
  std::vector<internal::PTy> deltas(PTy_bits);
  deltas[0] = delta;
  for (size_t i = 1; i < PTy_bits; ++i) {
    deltas[i] = deltas[i - 1] * internal::PTy(2);
  }
  // prepare for OT
  std::vector<std::array<uint128_t, 2>> ot_send_msgs(ot_num);
  ot_sender_->send_rot(absl::MakeSpan(ot_send_msgs));

  // Convert ROT to additive-COT
  auto send_msgs = std::vector<internal::PTy>(ot_num);
  for (size_t i = 0; i < ext_num; ++i) {
    // ot_block0 - ot_block1 + b
    const size_t offset = i * PTy_bits;
    for (size_t k = 0; k < PTy_bits; ++k) {
      send_msgs[offset + k] = internal::PTy(ot_send_msgs[offset + k][0]) -
                              internal::PTy(ot_send_msgs[offset + k][1]) +
                              deltas[k];
    }
  }

  // send
  conn->SendAsync(
      conn->NextRank(),
      yacl::ByteContainerView(send_msgs.data(),
                              send_msgs.size() * sizeof(internal::PTy)),
      "Beaver:BaseVole");

  for (size_t i = 0; i < num; ++i) {
    c[i] = internal::PTy::Zero();
    const size_t offset = i * PTy_bits;
    for (size_t j = 0; j < PTy_bits; ++j) {
      // c = block0
      c[i] = c[i] - internal::PTy(ot_send_msgs[offset + j][0]);
    }
  }

  auto extra_c = internal::PTy::Zero();
  {
    const size_t offset = num * PTy_bits;
    for (size_t j = 0; j < PTy_bits; ++j) {
      // c = block0
      extra_c = extra_c - internal::PTy(ot_send_msgs[offset + j][0]);
    }
  }

  // Consistency check;
  auto seed = conn->SyncSeed();
  auto coef = internal::op::Rand(seed, num);
  extra_c = extra_c + internal::op::InPro(absl::MakeSpan(coef), c);
  auto buf = conn->Recv(conn->NextRank(), "MalBaseVole");
  auto extra_ab =
      absl::MakeSpan(reinterpret_cast<internal::PTy *>(buf.data()), 2);
  YACL_ENFORCE(extra_ab[0] * delta + extra_ab[1] == extra_c);
}

void OtHelper::BaseVoleRecv(std::shared_ptr<Connection> conn,
                            absl::Span<internal::PTy> a,
                            absl::Span<internal::PTy> b) {
  const size_t num = a.size();
  YACL_ENFORCE(num == b.size());
  const size_t ext_num = num + 1;
  // bits in Public Type
  const size_t PTy_bits = sizeof(internal::PTy) * 8;
  // ot num = ext_num * bits of Public Type
  const size_t ot_num = ext_num * PTy_bits;

  auto ext_a = internal::op::Rand(ext_num);  // rand a
  auto choices = yacl::dynamic_bitset<uint128_t>(ot_num);
  memcpy(a.data(), ext_a.data(), num * sizeof(internal::PTy));
  memcpy(choices.data(), ext_a.data(), ext_num * sizeof(internal::PTy));

  std::vector<uint128_t> ot_recv_msgs(ot_num);
  ot_receiver_->recv_rot(absl::MakeSpan(ot_recv_msgs), choices);

  auto recv_buf = conn->Recv(conn->NextRank(), "Beaver:BaseVole");
  auto recv_span = absl::MakeConstSpan(
      reinterpret_cast<internal::PTy *>(recv_buf.data()), ot_num);

  for (size_t i = 0; i < num; ++i) {
    b[i] = internal::PTy::Zero();
    const size_t offset = i * PTy_bits;
    for (size_t j = 0; j < PTy_bits; ++j) {
      b[i] = b[i] - internal::PTy(ot_recv_msgs[offset + j]);
      if (choices[offset + j]) {
        b[i] = b[i] - internal::PTy(recv_span[offset + j]);
      }
    }
  }

  std::array<internal::PTy, 2> extra_ab;  // extra_a && extra_b
  extra_ab[0] = ext_a[num];
  extra_ab[1] = internal::PTy::Zero();
  {
    const size_t offset = num * PTy_bits;
    for (size_t j = 0; j < PTy_bits; ++j) {
      extra_ab[1] = extra_ab[1] - internal::PTy(ot_recv_msgs[offset + j]);
      if (choices[offset + j]) {
        extra_ab[1] = extra_ab[1] - internal::PTy(recv_span[offset + j]);
      }
    }
  }

  // Consistency check;
  auto seed = conn->SyncSeed();
  auto coef = internal::op::Rand(seed, num);
  extra_ab[0] = extra_ab[0] + internal::op::InPro(absl::MakeSpan(coef), a);
  extra_ab[1] = extra_ab[1] + internal::op::InPro(absl::MakeSpan(coef), b);

  conn->SendAsync(
      conn->NextRank(),
      yacl::ByteContainerView(extra_ab.data(), 2 * sizeof(internal::PTy)),
      "MalBaseVole");
}

// ---------------------
//  Shuffle
// ---------------------

void OtHelper::ShuffleSend(std::shared_ptr<Connection> conn,
                           absl::Span<const size_t> perm,
                           absl::Span<internal::PTy> delta) {
  const size_t num = perm.size();
  YACL_ENFORCE(delta.size() == num);
  const size_t ot_num = ym::Log2Ceil(num);
  const size_t required_ot = num * ot_num;

  std::vector<internal::PTy> a(num, internal::PTy(0));
  std::vector<internal::PTy> b(num, internal::PTy(0));
  std::vector<internal::PTy> opv(num);
  std::vector<uint128_t> punctured_msgs(num);

  std::vector<uint128_t> ot_buff(required_ot);
  yacl::dynamic_bitset<uint128_t> choices(required_ot);

  ot_receiver_->recv_rcot(absl::MakeSpan(ot_buff), choices);
  auto ot_store = yc::MakeOtRecvStore(choices, ot_buff);

  for (size_t i = 0; i < num; ++i) {
    auto ot_recv = ot_store.NextSlice(ot_num);
    yc::GywzOtExtRecv(conn, ot_recv, num, perm[i],
                      absl::MakeSpan(punctured_msgs));
    // break correlation
    yc::ParaCrHashInplace_128(absl::MakeSpan(punctured_msgs));
    punctured_msgs[perm[i]] = 0;

    std::transform(punctured_msgs.begin(), punctured_msgs.begin() + num,
                   opv.begin(),
                   [](uint128_t val) { return internal::PTy(val); });
    internal::op::Add(absl::MakeConstSpan(a), absl::MakeConstSpan(opv),
                      absl::MakeSpan(a));
    b[i] = std::reduce(opv.begin(), opv.begin() + num, internal::PTy(0),
                       std::plus<internal::PTy>());
  }
  for (size_t i = 0; i < num; ++i) {
    delta[i] = a[perm[i]] - b[i];
  }
}

void OtHelper::ShuffleRecv(std::shared_ptr<Connection> conn,
                           absl::Span<internal::PTy> a,
                           absl::Span<internal::PTy> b) {
  const size_t num = a.size();
  YACL_ENFORCE(b.size() == num);
  const size_t ot_num = ym::Log2Ceil(num);
  const size_t required_ot = num * ot_num;

  std::memset(a.data(), 0, num * sizeof(internal::PTy));
  std::vector<internal::PTy> opv(num);
  std::vector<uint128_t> all_msgs(num);

  std::vector<uint128_t> ot_buff(required_ot);
  ot_sender_->send_rcot(absl::MakeSpan(ot_buff));
  auto ot_store = yc::MakeCompactOtSendStore(ot_buff, ot_sender_->GetDelta());

  for (size_t i = 0; i < num; ++i) {
    auto ot_send = ot_store.NextSlice(ot_num);
    yc::GywzOtExtSend(conn, ot_send, num, absl::MakeSpan(all_msgs));
    // break correlation
    yc::ParaCrHashInplace_128(absl::MakeSpan(all_msgs));

    std::transform(all_msgs.begin(), all_msgs.begin() + num, opv.begin(),
                   [](uint128_t val) { return internal::PTy(val); });
    internal::op::Sub(absl::MakeConstSpan(a), absl::MakeConstSpan(opv),
                      absl::MakeSpan(a));
    b[i] = std::reduce(opv.begin(), opv.begin() + num, internal::PTy(0),
                       std::plus<internal::PTy>());
  }
}

}  // namespace mcpsi::ot
