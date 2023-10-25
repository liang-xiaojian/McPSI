#include "test/cr/fake_cr.h"

#include "test/utils/vec_op.h"

namespace test {

// register string
const std::string FakeCorrelation::id = std::string("FakeCorrelation");

void FakeCorrelation::BeaverTriple(absl::Span<internal::ATy> a,
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

void FakeCorrelation::RandomSet(absl::Span<internal::ATy> out) {
  const size_t num = out.size();
  auto rands = Rand(*ctx_->GetState<Prg>(), num);
  auto mac = ScalarMul(key_, absl::MakeSpan(rands));
  internal::Pack(absl::MakeConstSpan(rands), absl::MakeConstSpan(mac), out);
}

void FakeCorrelation::RandomGet(absl::Span<internal::ATy> out) {
  const size_t num = out.size();
  auto rands = Rand(*ctx_->GetState<Prg>(), num);
  auto mac = ScalarMul(key_, absl::MakeSpan(rands));
  auto zeros = Zeros(num);
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
  Add(absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(zeros.data()),
                          2 * num),
      absl::MakeConstSpan(reinterpret_cast<const internal::PTy*>(rands.data()),
                          2 * num),
      absl::MakeSpan(reinterpret_cast<internal::PTy*>(out.data()), 2 * num));
}

void FakeCorrelation::ShuffleSet(absl::Span<const size_t> perm,
                                 absl::Span<internal::PTy> delta) {
  const size_t size = delta.size();
  std::vector<internal::PTy> a(size);
  std::vector<internal::PTy> b(size);

  Rand(absl::MakeSpan(a));
  Rand(absl::MakeSpan(b));

  std::vector<internal::PTy> pi(size);
  for (size_t i = 0; i < size; ++i) {
    delta[i] = a[perm[i]] + b[i];
  }
  // delta = - \Pi(a) - b
  Neg(absl::MakeConstSpan(delta), absl::MakeSpan(delta));

  auto lctx = ctx_->GetLink();
  lctx->SendAsync(
      ctx_->NextRank(),
      yacl::ByteContainerView(a.data(), a.size() * sizeof(internal::PTy)),
      "send:a");
  lctx->SendAsync(
      ctx_->NextRank(),
      yacl::ByteContainerView(b.data(), b.size() * sizeof(internal::PTy)),
      "send:b");
}

void FakeCorrelation::ShuffleGet(absl::Span<internal::PTy> a,
                                 absl::Span<internal::PTy> b) {
  auto lctx = ctx_->GetLink();
  auto a_buf = lctx->Recv(ctx_->NextRank(), "send:a");
  auto b_buf = lctx->Recv(ctx_->NextRank(), "send:b");

  memcpy(a.data(), a_buf.data(), a_buf.size());
  memcpy(b.data(), b_buf.data(), b_buf.size());
}

}  // namespace test
