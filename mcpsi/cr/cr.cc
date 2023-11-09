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
