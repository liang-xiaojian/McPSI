#include "mcpsi/utils/vec_op.h"

#include "field.h"

namespace mcpsi {

// -------------------
//     Fp 64-bit
// -------------------

void op64::Add(absl::Span<const kFp64> lhs, absl::Span<const kFp64> rhs,
               absl::Span<kFp64> out) {
  const uint32_t size = out.size();
  YACL_ENFORCE(size == lhs.size());
  YACL_ENFORCE(size == rhs.size());

  std::transform(lhs.begin(), lhs.end(), rhs.begin(), out.begin(), std::plus());
}

void op64::Sub(absl::Span<const kFp64> lhs, absl::Span<const kFp64> rhs,
               absl::Span<kFp64> out) {
  const uint32_t size = out.size();
  YACL_ENFORCE(size == lhs.size());
  YACL_ENFORCE(size == rhs.size());

  std::transform(lhs.begin(), lhs.end(), rhs.begin(), out.begin(),
                 std::minus());
}

void op64::Mul(absl::Span<const kFp64> lhs, absl::Span<const kFp64> rhs,
               absl::Span<kFp64> out) {
  const uint32_t size = out.size();
  YACL_ENFORCE(size == lhs.size());
  YACL_ENFORCE(size == rhs.size());

  std::transform(lhs.begin(), lhs.end(), rhs.begin(), out.begin(),
                 std::multiplies());
}

void op64::ScalarMul(const kFp64 scalar, absl::Span<const kFp64> in,
                     absl::Span<kFp64> out) {
  YACL_ENFORCE(out.size() == in.size());
  std::transform(in.begin(), in.end(), out.begin(),
                 [&scalar](kFp64 val) { return scalar * val; });
}

void op64::Div(absl::Span<const kFp64> lhs, absl::Span<const kFp64> rhs,
               absl::Span<kFp64> out) {
  const uint32_t size = out.size();
  YACL_ENFORCE(size == lhs.size());
  YACL_ENFORCE(size == rhs.size());
  Inv(rhs, out);
  Mul(lhs, out, out);
}

void op64::Neg(absl::Span<const kFp64> in, absl::Span<kFp64> out) {
  YACL_ENFORCE(in.size() == out.size());
  std::transform(in.begin(), in.end(), out.begin(), kFp64::Neg);
}

template <size_t N>
kFp64 BatchInv(absl::Span<const kFp64> in, absl::Span<kFp64> out,
               kFp64 total = kFp64::One()) {
  auto inv = BatchInv<N - 1>(in, out, total * in[N - 1]);
  out[N - 1] = inv * total;
  return inv * in[N - 1];
}

template <>
kFp64 BatchInv<1>(absl::Span<const kFp64> in, absl::Span<kFp64> out,
                  kFp64 total) {
  auto inv = kFp64::Inv(total * in[0]);
  out[0] = total * inv;
  return inv * in[0];
}

// Batch Invert Optimize
void op64::Inv(absl::Span<const kFp64> in, absl::Span<kFp64> out) {
  const size_t size = out.size();
  YACL_ENFORCE(in.size() == size);
  size_t batch = size / 16;
  size_t bound = batch * 16;
  size_t remain = size - bound;
  for (size_t i = 0; i < batch; ++i) {
    BatchInv<16>(in.subspan(i * 16, 16), out.subspan(i * 16, 16));
  }
  switch (remain) {
#define KASE(T)                                               \
  case T:                                                     \
    BatchInv<T>(in.subspan(bound, T), out.subspan(bound, T)); \
    break;
    KASE(15);
    KASE(14);
    KASE(13);
    KASE(12);
    KASE(11);
    KASE(10);
    KASE(9);
    KASE(8);
    KASE(7);
    KASE(6);
    KASE(5);
    KASE(4);
    KASE(3);
    KASE(2);
    KASE(1);
#undef KASE
    case 0:
      break;
    default:
      YACL_ENFORCE(false, "Inv Error");
  }
}

void op64::Ones(absl::Span<kFp64> out) {
  const size_t size = out.size();
  for (uint32_t i = 0; i < size; ++i) {
    out[i] = kFp64::One();
  }
}

void op64::Zeros(absl::Span<kFp64> out) {
  const size_t size = out.size();
  for (uint32_t i = 0; i < size; ++i) {
    out[i] = kFp64::Zero();
  }
}

void op64::Rand(absl::Span<kFp64> out) {
  const uint32_t size = out.size();
  const uint64_t prime = kFp64::GetPrime();

  auto out64 =
      absl::MakeSpan(reinterpret_cast<uint64_t*>(out.data()), out.size());
  yacl::crypto::FillRand(out64, true);

  for (uint32_t i = 0; i < size; ++i) {
    out64[i] %= prime;
  }
}

void op64::Rand(yacl::crypto::Prg<uint8_t>& prg, absl::Span<kFp64> out) {
  const uint32_t size = out.size();
  const uint64_t prime = kFp64::GetPrime();

  auto out64 =
      absl::MakeSpan(reinterpret_cast<uint64_t*>(out.data()), out.size());
  prg.Fill(out64);

  for (uint32_t i = 0; i < size; ++i) {
    out64[i] %= prime;
  }
}

// -------------------
//     Fp 128-bit
// -------------------

void op128::Add(absl::Span<const kFp128> lhs, absl::Span<const kFp128> rhs,
                absl::Span<kFp128> out) {
  const uint32_t size = out.size();
  YACL_ENFORCE(size == lhs.size());
  YACL_ENFORCE(size == rhs.size());

  std::transform(lhs.begin(), lhs.end(), rhs.begin(), out.begin(), std::plus());
}

void op128::Sub(absl::Span<const kFp128> lhs, absl::Span<const kFp128> rhs,
                absl::Span<kFp128> out) {
  const uint32_t size = out.size();
  YACL_ENFORCE(size == lhs.size());
  YACL_ENFORCE(size == rhs.size());

  std::transform(lhs.begin(), lhs.end(), rhs.begin(), out.begin(),
                 std::minus());
}

void op128::Mul(absl::Span<const kFp128> lhs, absl::Span<const kFp128> rhs,
                absl::Span<kFp128> out) {
  const uint32_t size = out.size();
  YACL_ENFORCE(size == lhs.size());
  YACL_ENFORCE(size == rhs.size());

  std::transform(lhs.begin(), lhs.end(), rhs.begin(), out.begin(),
                 std::multiplies());
}

void op128::ScalarMul(const kFp128 scalar, absl::Span<const kFp128> in,
                      absl::Span<kFp128> out) {
  YACL_ENFORCE(out.size() == in.size());
  std::transform(in.begin(), in.end(), out.begin(),
                 [&scalar](kFp128 val) { return scalar * val; });
}

void op128::Div(absl::Span<const kFp128> lhs, absl::Span<const kFp128> rhs,
                absl::Span<kFp128> out) {
  const uint32_t size = out.size();
  YACL_ENFORCE(size == lhs.size());
  YACL_ENFORCE(size == rhs.size());
  Inv(rhs, out);
  Mul(lhs, out, out);
}

void op128::Neg(absl::Span<const kFp128> in, absl::Span<kFp128> out) {
  YACL_ENFORCE(in.size() == out.size());
  std::transform(in.begin(), in.end(), out.begin(), kFp128::Neg);
}

template <size_t N>
kFp128 BatchInv(absl::Span<const kFp128> in, absl::Span<kFp128> out,
                kFp128 total = kFp128::One()) {
  auto inv = BatchInv<N - 1>(in, out, total * in[N - 1]);
  out[N - 1] = inv * total;
  return inv * in[N - 1];
}

template <>
kFp128 BatchInv<1>(absl::Span<const kFp128> in, absl::Span<kFp128> out,
                   kFp128 total) {
  auto inv = kFp128::Inv(total * in[0]);
  out[0] = total * inv;
  return inv * in[0];
}

// Batch Invert Optimize
void op128::Inv(absl::Span<const kFp128> in, absl::Span<kFp128> out) {
  const size_t size = out.size();
  YACL_ENFORCE(in.size() == size);
  size_t batch = size / 16;
  size_t bound = batch * 16;
  size_t remain = size - bound;
  for (size_t i = 0; i < batch; ++i) {
    BatchInv<16>(in.subspan(i * 16, 16), out.subspan(i * 16, 16));
  }
  switch (remain) {
#define KASE(T)                                               \
  case T:                                                     \
    BatchInv<T>(in.subspan(bound, T), out.subspan(bound, T)); \
    break;
    KASE(15);
    KASE(14);
    KASE(13);
    KASE(12);
    KASE(11);
    KASE(10);
    KASE(9);
    KASE(8);
    KASE(7);
    KASE(6);
    KASE(5);
    KASE(4);
    KASE(3);
    KASE(2);
    KASE(1);
#undef KASE
    case 0:
      break;
    default:
      YACL_ENFORCE(false, "Inv Error");
  }
}

void op128::Ones(absl::Span<kFp128> out) {
  const size_t size = out.size();
  for (uint32_t i = 0; i < size; ++i) {
    out[i] = kFp128::One();
  }
}

void op128::Zeros(absl::Span<kFp128> out) {
  const size_t size = out.size();
  for (uint32_t i = 0; i < size; ++i) {
    out[i] = kFp128::Zero();
  }
}

void op128::Rand(absl::Span<kFp128> out) {
  const uint32_t size = out.size();
  const uint128_t prime = kFp128::GetPrime();

  auto out128 =
      absl::MakeSpan(reinterpret_cast<uint128_t*>(out.data()), out.size());
  yacl::crypto::FillRand(out128, true);

  for (uint32_t i = 0; i < size; ++i) {
    out128[i] %= prime;
  }
}

void op128::Rand(yacl::crypto::Prg<uint8_t>& prg, absl::Span<kFp128> out) {
  const uint32_t size = out.size();
  const uint128_t prime = kFp128::GetPrime();

  auto out128 =
      absl::MakeSpan(reinterpret_cast<uint128_t*>(out.data()), out.size());
  prg.Fill(out128);

  for (uint32_t i = 0; i < size; ++i) {
    out128[i] %= prime;
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

}  // namespace mcpsi
