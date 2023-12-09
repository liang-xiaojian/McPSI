#include "mcpsi/cr/fake_cr.h"

#include "mcpsi/utils/vec_op.h"

namespace mcpsi {

// register string
const std::string FakeCorrelation::id = std::string("FakeCorrelation");

void FakeCorrelation::BeaverTriple(absl::Span<internal::ATy> a,
                                   absl::Span<internal::ATy> b,
                                   absl::Span<internal::ATy> c) {
  const size_t num = c.size();
  YACL_ENFORCE(num == a.size());
  YACL_ENFORCE(num == b.size());

  auto a0 = internal::op::Rand(*ctx_->GetState<Prg>(), num);
  auto a1 = internal::op::Rand(*ctx_->GetState<Prg>(), num);
  auto b0 = internal::op::Rand(*ctx_->GetState<Prg>(), num);
  auto b1 = internal::op::Rand(*ctx_->GetState<Prg>(), num);
  auto c0 = internal::op::Rand(*ctx_->GetState<Prg>(), num);

  auto aa = internal::op::Add(absl::MakeConstSpan(a0), absl::MakeConstSpan(a1));
  auto bb = internal::op::Add(absl::MakeConstSpan(b0), absl::MakeConstSpan(b1));
  auto cc = internal::op::Mul(absl::MakeConstSpan(aa), absl::MakeConstSpan(bb));
  auto c1 = internal::op::Sub(absl::MakeConstSpan(cc), absl::MakeConstSpan(c0));

  auto a_mac = internal::op::ScalarMul(key_, absl::MakeConstSpan(aa));
  auto b_mac = internal::op::ScalarMul(key_, absl::MakeConstSpan(bb));
  auto c_mac = internal::op::ScalarMul(key_, absl::MakeConstSpan(cc));

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

void FakeCorrelation::RandomSet(absl::Span<internal::ATy> out) {
  const size_t num = out.size();
  auto rands = internal::op::Rand(*ctx_->GetState<Prg>(), num);
  auto mac = internal::op::ScalarMul(key_, absl::MakeSpan(rands));
  internal::Pack(absl::MakeConstSpan(rands), absl::MakeConstSpan(mac), out);
}

void FakeCorrelation::RandomGet(absl::Span<internal::ATy> out) {
  const size_t num = out.size();
  auto rands = internal::op::Rand(*ctx_->GetState<Prg>(), num);
  auto mac = internal::op::ScalarMul(key_, absl::MakeSpan(rands));
  auto zeros = internal::op::Zeros(num);
  internal::Pack(absl::MakeConstSpan(zeros), absl::MakeConstSpan(mac), out);
}

void FakeCorrelation::RandomAuth(absl::Span<internal::ATy> out) {
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

void FakeCorrelation::ShuffleSet(absl::Span<const size_t> perm,
                                 absl::Span<internal::PTy> delta) {
  const size_t num = delta.size();

  std::vector<uint128_t> seeds(2);
  ctx_->GetState<Prg>()->Fill(absl::MakeSpan(seeds));
  auto a = internal::op::Rand(seeds[0], num);
  auto b = internal::op::Rand(seeds[1], num);

  for (size_t i = 0; i < num; ++i) {
    delta[i] = a[perm[i]] + b[i];
  }
  // delta = - \Pi(a) - b
  internal::op::Neg(absl::MakeConstSpan(delta), absl::MakeSpan(delta));
}

void FakeCorrelation::ShuffleGet(absl::Span<internal::PTy> a,
                                 absl::Span<internal::PTy> b) {
  const size_t num = a.size();
  YACL_ENFORCE(num == b.size());

  std::vector<uint128_t> seeds(2);
  ctx_->GetState<Prg>()->Fill(absl::MakeSpan(seeds));
  auto a_buf = internal::op::Rand(seeds[0], num);
  auto b_buf = internal::op::Rand(seeds[1], num);

  memcpy(a.data(), a_buf.data(), num * sizeof(internal::PTy));
  memcpy(b.data(), b_buf.data(), num * sizeof(internal::PTy));
}

}  // namespace mcpsi
