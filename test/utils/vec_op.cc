#include "test/utils/vec_op.h"

#include "field.h"

namespace test {
void Add(absl::Span<const kFp64> lhs, absl::Span<const kFp64> rhs,
         absl::Span<kFp64> out) {
  const uint32_t size = out.size();
  YACL_ENFORCE(size == lhs.size());
  YACL_ENFORCE(size == rhs.size());

  for (uint32_t i = 0; i < size; ++i) {
    out[i] = lhs[i] + rhs[i];
  }
}

void Sub(absl::Span<const kFp64> lhs, absl::Span<const kFp64> rhs,
         absl::Span<kFp64> out) {
  const uint32_t size = out.size();
  YACL_ENFORCE(size == lhs.size());
  YACL_ENFORCE(size == rhs.size());

  for (uint32_t i = 0; i < size; ++i) {
    out[i] = lhs[i] - rhs[i];
  }
}

void Mul(absl::Span<const kFp64> lhs, absl::Span<const kFp64> rhs,
         absl::Span<kFp64> out) {
  const uint32_t size = out.size();
  YACL_ENFORCE(size == lhs.size());
  YACL_ENFORCE(size == rhs.size());

  for (uint32_t i = 0; i < size; ++i) {
    out[i] = lhs[i] * rhs[i];
  }
}

void ScalarMul(const kFp64 scalar, absl::Span<const kFp64> in,
               absl::Span<kFp64> out) {
  const uint32_t size = out.size();
  YACL_ENFORCE(size == in.size());

  for (uint32_t i = 0; i < size; ++i) {
    out[i] = scalar * in[i];
  }
}

void Div(absl::Span<const kFp64> lhs, absl::Span<const kFp64> rhs,
         absl::Span<kFp64> out) {
  const uint32_t size = out.size();
  YACL_ENFORCE(size == lhs.size());
  YACL_ENFORCE(size == rhs.size());

  for (uint32_t i = 0; i < size; ++i) {
    out[i] = lhs[i] / rhs[i];
  }
}

void Neg(absl::Span<const kFp64> in, absl::Span<kFp64> out) {
  const size_t size = out.size();
  YACL_ENFORCE(in.size() == size);
  for (uint32_t i = 0; i < size; ++i) {
    out[i] = kFp64::Neg(in[i]);
  }
}

void Inv(absl::Span<const kFp64> in, absl::Span<kFp64> out) {
  const size_t size = out.size();
  YACL_ENFORCE(in.size() == size);
  for (uint32_t i = 0; i < size; ++i) {
    out[i] = kFp64::Inv(in[i]);
  }
}

void Ones(absl::Span<kFp64> out) {
  const size_t size = out.size();
  for (uint32_t i = 0; i < size; ++i) {
    out[i] = kFp64::One();
  }
}

void Zeros(absl::Span<kFp64> out) {
  const size_t size = out.size();
  for (uint32_t i = 0; i < size; ++i) {
    out[i] = kFp64::Zero();
  }
}

void Rand(absl::Span<kFp64> out) {
  const uint32_t size = out.size();
  const uint64_t prime = kFp64::GetPrime();

  auto out64 =
      absl::MakeSpan(reinterpret_cast<uint64_t*>(out.data()), out.size());
  yacl::crypto::FillRand(out64, true);

  for (uint32_t i = 0; i < size; ++i) {
    out64[i] %= prime;
  }
}

void Rand(yacl::crypto::Prg<uint8_t>& prg, absl::Span<kFp64> out) {
  const uint32_t size = out.size();
  const uint64_t prime = kFp64::GetPrime();

  auto out64 =
      absl::MakeSpan(reinterpret_cast<uint64_t*>(out.data()), out.size());
  prg.Fill(out64);

  for (uint32_t i = 0; i < size; ++i) {
    out64[i] %= prime;
  }
}

std::vector<size_t> GenPerm(uint32_t num) {
  std::vector<size_t> perm(num);
  for (size_t i = 0; i < num; ++i) {
    perm[i] = i;
  }
  // std::shuffle
  std::random_device rd;
  std::shuffle(perm.begin(), perm.end(), std::mt19937(rd()));
  return perm;
}

}  // namespace test
