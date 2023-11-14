#include "mcpsi/cr/cr.h"

#include "mcpsi/cr/fake_cr.h"
#include "mcpsi/cr/utils/ot_helper.h"
#include "mcpsi/ss/type.h"
#include "mcpsi/utils/vec_op.h"

namespace mcpsi {

// register string
const std::string Correlation::id = std::string("Correlation");

void Correlation::BeaverTriple(absl::Span<internal::ATy> a,
                               absl::Span<internal::ATy> b,
                               absl::Span<internal::ATy> c) {
  const size_t num = c.size();
  YACL_ENFORCE(num == a.size());
  YACL_ENFORCE(num == b.size());

  auto p_a = Zeros(num * 2);
  auto p_b = Zeros(num * 2);
  auto p_c = Zeros(num * 2);

  auto conn = ctx_->GetConnection();
  ot::OtHelper(ot_sender_, ot_receiver_)
      .BeaverTriple(conn, absl::MakeSpan(p_a), absl::MakeSpan(p_b),
                    absl::MakeSpan(p_c));

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

void Correlation::RandomSet(absl::Span<internal::ATy> out) {
  const size_t num = out.size();
  std::vector<internal::PTy> a(num);
  std::vector<internal::PTy> b(num);
  // a * remote_key + b = remote_c
  vole_receiver_->rrecv(absl::MakeSpan(a), absl::MakeSpan(b));
  // mac = a * key_
  auto mac = ScalarMul(key_, absl::MakeConstSpan(a));
  // a's mac = a * local_key - b
  Sub(absl::MakeConstSpan(mac), absl::MakeConstSpan(b), absl::MakeSpan(mac));
  // Pack
  internal::Pack(absl::MakeConstSpan(a), absl::MakeConstSpan(mac),
                 absl::MakeSpan(out));
}

void Correlation::RandomGet(absl::Span<internal::ATy> out) {
  const size_t num = out.size();
  std::vector<internal::PTy> c(num);
  // remote_a * key_ + remote_b = c
  vole_sender_->rsend(absl::MakeSpan(c));
  // Pack
  auto zeros = Zeros(num);
  internal::Pack(absl::MakeConstSpan(zeros), absl::MakeConstSpan(c),
                 absl::MakeSpan(out));
}

void Correlation::RandomAuth(absl::Span<internal::ATy> out) {
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
  Add(absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(zeros.data()),
                          2 * num),
      absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(rands.data()),
                          2 * num),
      absl::MakeSpan(reinterpret_cast<internal::PTy*>(out.data()), 2 * num));
}

// use fake correlation to generate Shuffle
// TODO: remove fake cr
void Correlation::ShuffleSet(absl::Span<const size_t> perm,
                             absl::Span<internal::PTy> delta) {
  fake_cr_ptr_->ShuffleSet(perm, delta);
}

void Correlation::ShuffleGet(absl::Span<internal::PTy> a,
                             absl::Span<internal::PTy> b) {
  fake_cr_ptr_->ShuffleGet(a, b);
}

}  // namespace mcpsi
