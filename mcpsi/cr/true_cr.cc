#include "mcpsi/cr/true_cr.h"

#include "mcpsi/cr/utils/ot_helper.h"
#include "mcpsi/ss/type.h"
#include "mcpsi/utils/vec_op.h"

namespace mcpsi {

void TrueCorrelation::BeaverTriple(absl::Span<internal::ATy> a,
                                   absl::Span<internal::ATy> b,
                                   absl::Span<internal::ATy> c) {
  const size_t num = c.size();
  YACL_ENFORCE(num == a.size());
  YACL_ENFORCE(num == b.size());

  auto p_abcAC = internal::op::Zeros(num * 5);
  auto p_abcAC_span = absl::MakeSpan(p_abcAC);
  auto p_a = p_abcAC_span.subspan(0 * num, num);
  auto p_b = p_abcAC_span.subspan(1 * num, num);
  auto p_c = p_abcAC_span.subspan(2 * num, num);
  auto p_A = p_abcAC_span.subspan(3 * num, num);
  auto p_C = p_abcAC_span.subspan(4 * num, num);

  auto conn = ctx_->GetConnection();
  ot::OtHelper(ot_sender_, ot_receiver_)
      .BeaverTripleExtend(conn, p_a, p_b, p_c, p_A, p_C);

  std::vector<internal::ATy> auth_abcAC(num * 5);
  auto auth_abcAC_span = absl::MakeSpan(auth_abcAC);

  std::vector<internal::ATy> remote_auth_abcAC(num * 5);
  auto remote_auth_abcAC_span = absl::MakeSpan(remote_auth_abcAC);

  // TODO: fix it
  // It need choose-and-cut strategy
  if (ctx_->GetRank() == 0) {
    AuthSet(p_abcAC_span, auth_abcAC_span);
    AuthGet(remote_auth_abcAC_span);
  } else {
    AuthGet(remote_auth_abcAC_span);
    AuthSet(p_abcAC_span, auth_abcAC_span);
  }

  // length double
  internal::op::Add(
      absl::MakeConstSpan(
          reinterpret_cast<const internal::PTy*>(auth_abcAC.data()), 10 * num),
      absl::MakeConstSpan(
          reinterpret_cast<const internal::PTy*>(remote_auth_abcAC.data()),
          10 * num),
      absl::MakeSpan(reinterpret_cast<internal::PTy*>(auth_abcAC.data()),
                     10 * num));

  // return value
  auto auth_a = auth_abcAC_span.subspan(0 * num, num);
  auto auth_b = auth_abcAC_span.subspan(1 * num, num);
  auto auth_c = auth_abcAC_span.subspan(2 * num, num);
  memcpy(a.data(), auth_a.data(), num * sizeof(internal::ATy));
  memcpy(b.data(), auth_b.data(), num * sizeof(internal::ATy));
  memcpy(c.data(), auth_c.data(), num * sizeof(internal::ATy));

  // consistency check
  auto auth_A = auth_abcAC_span.subspan(3 * num, num);
  auto auth_C = auth_abcAC_span.subspan(4 * num, num);
  auto seed = conn->SyncSeed();
  auto p_coef = internal::op::Rand(seed, num);
  std::vector<internal::ATy> coef(num, {0, 0});
  std::transform(p_coef.cbegin(), p_coef.cend(), coef.begin(),
                 [](const internal::PTy& val) -> internal::ATy {
                   return {val, val};
                 });

  internal::op::Mul(
      absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(auth_A.data()),
                          2 * num),
      absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(coef.data()),
                          2 * num),
      absl::MakeSpan(reinterpret_cast<internal::PTy*>(auth_A.data()), 2 * num));

  internal::op::Add(
      absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(auth_a.data()),
                          2 * num),
      absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(auth_A.data()),
                          2 * num),
      absl::MakeSpan(reinterpret_cast<internal::PTy*>(auth_A.data()), 2 * num));

  internal::op::Mul(
      absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(auth_C.data()),
                          2 * num),
      absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(coef.data()),
                          2 * num),
      absl::MakeSpan(reinterpret_cast<internal::PTy*>(auth_C.data()), 2 * num));

  internal::op::Add(
      absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(auth_c.data()),
                          2 * num),
      absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(auth_C.data()),
                          2 * num),
      absl::MakeSpan(reinterpret_cast<internal::PTy*>(auth_C.data()), 2 * num));

  // internal::PTy type
  auto aA_cC = OpenAndCheck(auth_abcAC_span.subspan(3 * num, 2 * num));
  auto aA_cC_span = absl::MakeSpan(aA_cC);
  auto aA = aA_cC_span.subspan(0, num);
  auto cC = aA_cC_span.subspan(num, num);

  internal::op::Mul(aA, p_b, aA);

  auto buf = conn->Exchange(
      yacl::ByteContainerView(aA.data(), num * sizeof(internal::PTy)));
  YACL_ENFORCE(static_cast<uint64_t>(buf.size()) ==
               num * sizeof(internal::PTy));
  auto remote_aA =
      absl::MakeSpan(reinterpret_cast<internal::PTy*>(buf.data()), num);

  internal::op::Add(aA, remote_aA, aA);
  for (size_t i = 0; i < num; ++i) {
    YACL_ENFORCE(cC[i] == aA[i], "{} : cC is {}", i, cC[i].GetVal());
  }
}

void TrueCorrelation::AuthSet(absl::Span<const internal::PTy> in,
                              absl::Span<internal::ATy> out) {
  RandomSet(out);
  auto [val, mac] = internal::Unpack(out);
  // val = in - val
  internal::op::Sub(absl::MakeConstSpan(in), absl::MakeConstSpan(val),
                    absl::MakeSpan(val));

  auto conn = ctx_->GetConnection();
  conn->SendAsync(
      conn->NextRank(),
      yacl::ByteContainerView(val.data(), val.size() * sizeof(internal::PTy)),
      "AuthSet");

  internal::op::Add(
      absl::MakeConstSpan(mac),
      absl::MakeConstSpan(internal::op::ScalarMul(key_, absl::MakeSpan(val))),
      absl::MakeSpan(mac));
  auto ret = internal::Pack(in, mac);
  memcpy(out.data(), ret.data(), out.size() * sizeof(internal::ATy));
}

void TrueCorrelation::AuthGet(absl::Span<internal::ATy> out) {
  RandomGet(out);
  // val = 0
  auto [val, mac] = internal::Unpack(out);

  auto conn = ctx_->GetConnection();
  auto recv_buf = conn->Recv(conn->NextRank(), "AuthSet");

  auto diff = absl::MakeSpan(reinterpret_cast<internal::PTy*>(recv_buf.data()),
                             out.size());

  internal::op::Add(absl::MakeConstSpan(mac),
                    absl::MakeConstSpan(internal::op::ScalarMul(key_, diff)),
                    absl::MakeSpan(mac));
  auto ret = internal::Pack(val, mac);
  memcpy(out.data(), ret.data(), ret.size() * sizeof(internal::ATy));
}

void TrueCorrelation::RandomSet(absl::Span<internal::ATy> out) {
  const size_t num = out.size();
  std::vector<internal::PTy> a(num);
  std::vector<internal::PTy> b(num);
  // a * remote_key + b = remote_c
  vole_receiver_->rrecv(absl::MakeSpan(a), absl::MakeSpan(b));
  // mac = a * key_
  auto mac = internal::op::ScalarMul(key_, absl::MakeConstSpan(a));
  // a's mac = a * local_key - b
  internal::op::Sub(absl::MakeConstSpan(mac), absl::MakeConstSpan(b),
                    absl::MakeSpan(mac));
  // Pack
  internal::Pack(absl::MakeConstSpan(a), absl::MakeConstSpan(mac),
                 absl::MakeSpan(out));
}

void TrueCorrelation::RandomGet(absl::Span<internal::ATy> out) {
  const size_t num = out.size();
  std::vector<internal::PTy> c(num);
  // remote_a * key_ + remote_b = c
  vole_sender_->rsend(absl::MakeSpan(c));
  // Pack
  auto zeros = internal::op::Zeros(num);
  internal::Pack(absl::MakeConstSpan(zeros), absl::MakeConstSpan(c),
                 absl::MakeSpan(out));
}

void TrueCorrelation::RandomAuth(absl::Span<internal::ATy> out) {
  const size_t num = out.size();
  std::vector<internal::ATy> zeros(num);
  std::vector<internal::ATy> rands(num);
  if (ctx_->GetRank() == 0) {
    RandomSet(absl::MakeSpan(rands));
    RandomGet(absl::MakeSpan(zeros));
  } else {
    RandomGet(absl::MakeSpan(zeros));
    RandomSet(absl::MakeSpan(rands));
  }
  internal::op::Add(
      absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(zeros.data()),
                          2 * num),
      absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(rands.data()),
                          2 * num),
      absl::MakeSpan(reinterpret_cast<internal::PTy*>(out.data()), 2 * num));
}

// TODO: acheive malicious secure
void TrueCorrelation::ShuffleSet(absl::Span<const size_t> perm,
                                 absl::Span<internal::PTy> delta,
                                 size_t repeat) {
  auto conn = ctx_->GetConnection();
  const size_t batch_size = perm.size();
  const size_t full_size = delta.size();
  YACL_ENFORCE(full_size == batch_size * repeat);

  ot::OtHelper(ot_sender_, ot_receiver_).ShuffleSend(conn, perm, delta, repeat);
}

void TrueCorrelation::ShuffleGet(absl::Span<internal::PTy> a,
                                 absl::Span<internal::PTy> b, size_t repeat) {
  auto conn = ctx_->GetConnection();

  const size_t full_size = a.size();
  const size_t batch_size = a.size() / repeat;
  YACL_ENFORCE(full_size == b.size());
  YACL_ENFORCE(full_size == batch_size * repeat);

  ot::OtHelper(ot_sender_, ot_receiver_).ShuffleRecv(conn, a, b, repeat);
}

// Copy from A2P
std::vector<internal::PTy> TrueCorrelation::OpenAndCheck(
    absl::Span<const internal::ATy> in) {
  const size_t size = in.size();
  auto [val, mac] = internal::Unpack(absl::MakeSpan(in));
  auto conn = ctx_->GetConnection();
  auto val_bv =
      yacl::ByteContainerView(val.data(), size * sizeof(internal::PTy));
  std::vector<internal::PTy> real_val(size);

  auto buf = conn->Exchange(val_bv);
  internal::op::Add(
      absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(buf.data()),
                          size),
      absl::MakeConstSpan(val), absl::MakeSpan(real_val));

  // Generate Sync Seed After open Value
  auto sync_seed = conn->SyncSeed();
  auto coef = internal::op::Rand(sync_seed, size);
  // linear combination
  auto real_val_affine =
      internal::op::InPro(absl::MakeSpan(coef), absl::MakeSpan(real_val));
  auto mac_affine =
      internal::op::InPro(absl::MakeSpan(coef), absl::MakeSpan(mac));

  auto zero_mac = mac_affine - real_val_affine * key_;

  auto remote_mac_uint = conn->ExchangeWithCommit(zero_mac.GetVal());
  YACL_ENFORCE(zero_mac + internal::PTy(remote_mac_uint) ==
               internal::PTy::Zero());
  return real_val;
}
}  // namespace mcpsi
