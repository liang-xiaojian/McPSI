#include "mcpsi/cr/cr.h"

#include "mcpsi/cr/fake_cr.h"
#include "mcpsi/ss/type.h"
#include "mcpsi/utils/vec_op.h"

namespace mcpsi {

namespace {
constexpr size_t kExtFactor = 1;
}

// register string
const std::string Correlation::id = std::string("Correlation");

void Correlation::MulPPSender(absl::Span<internal::PTy> a,
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

  auto conn = ctx_->GetConnection();
  auto recv_buf = conn->Recv(conn->NextRank(), "Beaver:MulPP");
  auto recv_span = absl::MakeConstSpan(
      reinterpret_cast<internal::PTy*>(recv_buf.data()), ot_num);

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

void Correlation::MulPPReceiver(absl::Span<internal::PTy> b,
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
  auto conn = ctx_->GetConnection();
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

void Correlation::BeaverTriple(absl::Span<internal::ATy> a,
                               absl::Span<internal::ATy> b,
                               absl::Span<internal::ATy> c) {
  const size_t num = c.size();
  YACL_ENFORCE(num == a.size());
  YACL_ENFORCE(num == b.size());

  auto p_a = Zeros(num * 2);
  auto p_b = Zeros(num * 2);
  auto p_c0 = Zeros(num * 2);
  auto p_c1 = Zeros(num * 2);
  if (ctx_->GetRank() == 0) {
    MulPPSender(absl::MakeSpan(p_a), absl::MakeSpan(p_c0));
    MulPPReceiver(absl::MakeSpan(p_b), absl::MakeSpan(p_c1));
  } else {
    MulPPReceiver(absl::MakeSpan(p_b), absl::MakeSpan(p_c0));
    MulPPSender(absl::MakeSpan(p_a), absl::MakeSpan(p_c1));
  }
  auto p_c = Add(absl::MakeConstSpan(p_c0), absl::MakeConstSpan(p_c1));

  std::vector<internal::ATy> auth_a(num);
  std::vector<internal::ATy> auth_b(num);
  std::vector<internal::ATy> auth_c(num);
  // TODO: fix it
  // It need choose-and-cut strategy
  if (ctx_->GetRank() == 0) {
    AuthSet(absl::MakeSpan(p_a).subspan(0, num), absl::MakeSpan(auth_a));
    AuthSet(absl::MakeSpan(p_b).subspan(0, num), absl::MakeSpan(auth_b));
    AuthSet(absl::MakeSpan(p_c).subspan(0, num), absl::MakeSpan(auth_c));
  } else {
    AuthGet(absl::MakeSpan(auth_a));
    AuthGet(absl::MakeSpan(auth_b));
    AuthGet(absl::MakeSpan(auth_c));
  }

  memcpy(a.data(), auth_a.data(), num * sizeof(internal::ATy));
  memcpy(b.data(), auth_b.data(), num * sizeof(internal::ATy));
  memcpy(c.data(), auth_c.data(), num * sizeof(internal::ATy));

  if (ctx_->GetRank() != 0) {
    AuthSet(absl::MakeSpan(p_a).subspan(0, num), absl::MakeSpan(auth_a));
    AuthSet(absl::MakeSpan(p_b).subspan(0, num), absl::MakeSpan(auth_b));
    AuthSet(absl::MakeSpan(p_c).subspan(0, num), absl::MakeSpan(auth_c));
  } else {
    AuthGet(absl::MakeSpan(auth_a));
    AuthGet(absl::MakeSpan(auth_b));
    AuthGet(absl::MakeSpan(auth_c));
  }

  Add(absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(a.data()),
                          2 * num),
      absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(auth_a.data()),
                          2 * num),
      absl::MakeSpan(reinterpret_cast<internal::PTy*>(a.data()), 2 * num));
  Add(absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(b.data()),
                          2 * num),
      absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(auth_b.data()),
                          2 * num),
      absl::MakeSpan(reinterpret_cast<internal::PTy*>(b.data()), 2 * num));

  Add(absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(c.data()),
                          2 * num),
      absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(auth_c.data()),
                          2 * num),
      absl::MakeSpan(reinterpret_cast<internal::PTy*>(c.data()), 2 * num));

  auto p_ab = Mul(absl::MakeConstSpan(p_a), absl::MakeConstSpan(p_b));
  std::vector<internal::ATy> auth_ab(num);
  if (ctx_->GetRank() == 0) {
    AuthSet(absl::MakeConstSpan(p_ab).subspan(0, num), absl::MakeSpan(auth_ab));
  } else {
    AuthGet(absl::MakeSpan(auth_ab));
  }

  Add(absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(c.data()),
                          2 * num),
      absl::MakeConstSpan(
          reinterpret_cast<const internal::PTy*>(auth_ab.data()), 2 * num),
      absl::MakeSpan(reinterpret_cast<internal::PTy*>(c.data()), 2 * num));

  if (ctx_->GetRank() != 0) {
    AuthSet(absl::MakeConstSpan(p_ab).subspan(0, num), absl::MakeSpan(auth_ab));
  } else {
    AuthGet(absl::MakeSpan(auth_ab));
  }

  Add(absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(c.data()),
                          2 * num),
      absl::MakeConstSpan(
          reinterpret_cast<const internal::PTy*>(auth_ab.data()), 2 * num),
      absl::MakeSpan(reinterpret_cast<internal::PTy*>(c.data()), 2 * num));
}

void Correlation::AuthSet(absl::Span<const internal::PTy> in,
                          absl::Span<internal::ATy> out) {
  RandomSet(out);
  auto [val, mac] = internal::Unpack(out);
  // val = in - val
  Sub(absl::MakeConstSpan(in), absl::MakeConstSpan(val), absl::MakeSpan(val));

  auto conn = ctx_->GetConnection();
  conn->SendAsync(
      conn->NextRank(),
      yacl::ByteContainerView(val.data(), val.size() * sizeof(internal::PTy)),
      "AuthSet");

  Add(absl::MakeConstSpan(mac),
      absl::MakeConstSpan(ScalarMul(key_, absl::MakeSpan(val))),
      absl::MakeSpan(mac));
  auto ret = internal::Pack(in, mac);
  memcpy(out.data(), ret.data(), out.size() * sizeof(internal::ATy));
}

void Correlation::AuthGet(absl::Span<internal::ATy> out) {
  RandomGet(out);
  // val = 0
  auto [val, mac] = internal::Unpack(out);

  auto conn = ctx_->GetConnection();
  auto recv_buf = conn->Recv(conn->NextRank(), "AuthSet");

  auto diff = absl::MakeSpan(reinterpret_cast<internal::PTy*>(recv_buf.data()),
                             out.size());

  Add(absl::MakeConstSpan(mac), absl::MakeConstSpan(ScalarMul(key_, diff)),
      absl::MakeSpan(mac));
  auto ret = internal::Pack(val, mac);
  memcpy(out.data(), ret.data(), ret.size() * sizeof(internal::ATy));
}

// use fake correlation to generate RandomAuth && Shuffle
// TODO: remote fake cr
void Correlation::RandomSet(absl::Span<internal::ATy> out) {
  fake_cr_ptr_->RandomSet(out);
}

void Correlation::RandomGet(absl::Span<internal::ATy> out) {
  fake_cr_ptr_->RandomGet(out);
}

void Correlation::RandomAuth(absl::Span<internal::ATy> out) {
  fake_cr_ptr_->RandomAuth(out);
}

void Correlation::ShuffleSet(absl::Span<const size_t> perm,
                             absl::Span<internal::PTy> delta) {
  fake_cr_ptr_->ShuffleSet(perm, delta);
}

void Correlation::ShuffleGet(absl::Span<internal::PTy> a,
                             absl::Span<internal::PTy> b) {
  fake_cr_ptr_->ShuffleGet(a, b);
}

}  // namespace mcpsi
