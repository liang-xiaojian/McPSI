#include "mcpsi/cr/fake_cr.h"
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

  auto a0 = Rand(*ctx_->GetState<Prg>(), num);
  auto a1 = Rand(*ctx_->GetState<Prg>(), num);
  auto b0 = Rand(*ctx_->GetState<Prg>(), num);
  auto b1 = Rand(*ctx_->GetState<Prg>(), num);
  auto c0 = Rand(*ctx_->GetState<Prg>(), num);

  auto aa = Add(absl::MakeConstSpan(a0), absl::MakeConstSpan(a1));
  auto bb = Add(absl::MakeConstSpan(b0), absl::MakeConstSpan(b1));
  auto cc = Mul(absl::MakeConstSpan(aa), absl::MakeConstSpan(bb));
  auto c1 = Sub(absl::MakeConstSpan(cc), absl::MakeConstSpan(c0));

  auto a_mac = ScalarMul(key_, absl::MakeConstSpan(aa));
  auto b_mac = ScalarMul(key_, absl::MakeConstSpan(bb));
  auto c_mac = ScalarMul(key_, absl::MakeConstSpan(cc));

  if (ctx_->GetRank() == 0) {
    internal::Pack(absl::MakeConstSpan(a0), absl::MakeConstSpan(a_mac), a);
    internal::Pack(absl::MakeConstSpan(b0), absl::MakeConstSpan(b_mac), b);
    internal::Pack(absl::MakeConstSpan(c0), absl::MakeConstSpan(c_mac), c);
  } else {
    internal::Pack(absl::MakeConstSpan(a1), absl::MakeConstSpan(a_mac), a);
    internal::Pack(absl::MakeConstSpan(b1), absl::MakeConstSpan(b_mac), b);
    internal::Pack(absl::MakeConstSpan(c1), absl::MakeConstSpan(c_mac), c);
  }
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
