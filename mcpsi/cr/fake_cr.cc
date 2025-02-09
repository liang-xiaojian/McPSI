#include "mcpsi/cr/fake_cr.h"

#include "mcpsi/utils/vec_op.h"

namespace mcpsi {

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

void FakeCorrelation::BeaverTripleSet(absl::Span<internal::ATy> a,
                                      absl::Span<internal::ATy> b,
                                      absl::Span<internal::ATy> c) {
  const size_t num = c.size();
  YACL_ENFORCE(num == a.size());
  YACL_ENFORCE(num == b.size());

  auto a0 = internal::op::Rand(*ctx_->GetState<Prg>(), num);
  auto a1 = internal::op::Rand(*ctx_->GetState<Prg>(), num);
  auto bb = internal::op::Rand(*ctx_->GetState<Prg>(), num);
  auto c0 = internal::op::Rand(*ctx_->GetState<Prg>(), num);

  auto aa = internal::op::Add(absl::MakeConstSpan(a0), absl::MakeConstSpan(a1));
  auto cc = internal::op::Mul(absl::MakeConstSpan(aa), absl::MakeConstSpan(bb));
  auto c1 = internal::op::Sub(absl::MakeConstSpan(cc), absl::MakeConstSpan(c0));

  auto a_mac = internal::op::ScalarMul(key_, absl::MakeConstSpan(aa));
  auto b_mac = internal::op::ScalarMul(key_, absl::MakeConstSpan(bb));
  auto c_mac = internal::op::ScalarMul(key_, absl::MakeConstSpan(cc));

  internal::Pack(absl::MakeConstSpan(a0), absl::MakeConstSpan(a_mac), a);
  internal::Pack(absl::MakeConstSpan(bb), absl::MakeConstSpan(b_mac), b);
  internal::Pack(absl::MakeConstSpan(c0), absl::MakeConstSpan(c_mac), c);
}

void FakeCorrelation::BeaverTripleGet(absl::Span<internal::ATy> a,
                                      absl::Span<internal::ATy> b,
                                      absl::Span<internal::ATy> c) {
  const size_t num = c.size();
  YACL_ENFORCE(num == a.size());
  YACL_ENFORCE(num == b.size());

  auto a0 = internal::op::Rand(*ctx_->GetState<Prg>(), num);
  auto a1 = internal::op::Rand(*ctx_->GetState<Prg>(), num);
  auto bb = internal::op::Rand(*ctx_->GetState<Prg>(), num);
  auto c0 = internal::op::Rand(*ctx_->GetState<Prg>(), num);

  auto aa = internal::op::Add(absl::MakeConstSpan(a0), absl::MakeConstSpan(a1));
  auto cc = internal::op::Mul(absl::MakeConstSpan(aa), absl::MakeConstSpan(bb));
  auto c1 = internal::op::Sub(absl::MakeConstSpan(cc), absl::MakeConstSpan(c0));

  auto a_mac = internal::op::ScalarMul(key_, absl::MakeConstSpan(aa));
  auto b_mac = internal::op::ScalarMul(key_, absl::MakeConstSpan(bb));
  auto c_mac = internal::op::ScalarMul(key_, absl::MakeConstSpan(cc));

  auto zeros = internal::op::Zeros(num);

  internal::Pack(absl::MakeConstSpan(a1), absl::MakeConstSpan(a_mac), a);
  internal::Pack(absl::MakeConstSpan(zeros), absl::MakeConstSpan(b_mac), b);
  internal::Pack(absl::MakeConstSpan(c1), absl::MakeConstSpan(c_mac), c);
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
                                 absl::Span<internal::PTy> delta,
                                 size_t repeat) {
  const size_t batch_size = perm.size();
  const size_t full_size = delta.size();
  YACL_ENFORCE(batch_size * repeat == full_size);

  std::vector<uint128_t> seeds(2);
  ctx_->GetState<Prg>()->Fill(absl::MakeSpan(seeds));
  auto a = internal::op::Rand(seeds[0], full_size);
  auto b = internal::op::Rand(seeds[1], full_size);

  for (size_t offset = 0; offset < full_size; offset += batch_size) {
    for (size_t i = 0; i < batch_size; ++i) {
      delta[offset + i] = a[offset + perm[i]] + b[offset + i];
    }
  }
  // delta = - \Pi(a) - b
  internal::op::Neg(absl::MakeConstSpan(delta), absl::MakeSpan(delta));
}

void FakeCorrelation::ShuffleGet(absl::Span<internal::PTy> a,
                                 absl::Span<internal::PTy> b, size_t repeat) {
  const size_t full_size = a.size();
  const size_t batch_size = full_size / repeat;
  YACL_ENFORCE(full_size == b.size());
  YACL_ENFORCE(full_size == batch_size * repeat);

  std::vector<uint128_t> seeds(2);
  ctx_->GetState<Prg>()->Fill(absl::MakeSpan(seeds));
  auto a_buf = internal::op::Rand(seeds[0], full_size);
  auto b_buf = internal::op::Rand(seeds[1], full_size);

  memcpy(a.data(), a_buf.data(), full_size * sizeof(internal::PTy));
  memcpy(b.data(), b_buf.data(), full_size * sizeof(internal::PTy));
}

}  // namespace mcpsi
