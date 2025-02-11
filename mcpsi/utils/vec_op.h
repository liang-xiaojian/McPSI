#pragma once
#include <algorithm>
#include <random>
#include <vector>

#include "field.h"
#include "mcpsi/utils/field.h"
#include "yacl/crypto/tools/prg.h"
#include "yacl/crypto/utils/rand.h"
#include "yacl/math/mpint/mp_int.h"
#include "yacl/utils/parallel.h"

namespace mcpsi {

class op64 {
 public:
  static void Add(absl::Span<const kFp64> lhs, absl::Span<const kFp64> rhs,
                  absl::Span<kFp64> out);

  static void Sub(absl::Span<const kFp64> lhs, absl::Span<const kFp64> rhs,
                  absl::Span<kFp64> out);

  static void Mul(absl::Span<const kFp64> lhs, absl::Span<const kFp64> rhs,
                  absl::Span<kFp64> out);

  static void ScalarMul(const kFp64 scalar, absl::Span<const kFp64> in,
                        absl::Span<kFp64> out);

  // Implement : Mul(lsh , Inv(rhs))
  static void Div(absl::Span<const kFp64> lhs, absl::Span<const kFp64> rhs,
                  absl::Span<kFp64> out);

  static void Neg(absl::Span<const kFp64> in, absl::Span<kFp64> out);

  // fast batch inv
  static void Inv(absl::Span<const kFp64> in, absl::Span<kFp64> out);
  static void Ones(absl::Span<kFp64> out);
  static void Zeros(absl::Span<kFp64> out);
  static void Rand(absl::Span<kFp64> out);
  static void Rand(yacl::crypto::Prg<uint8_t>& prg, absl::Span<kFp64> out);

  static std::vector<kFp64> inline Add(absl::Span<const kFp64> lhs,
                                       absl::Span<const kFp64> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    std::vector<kFp64> ret(lhs.size());
    Add(lhs, rhs, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp64> inline Sub(absl::Span<const kFp64> lhs,
                                       absl::Span<const kFp64> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    std::vector<kFp64> ret(lhs.size());
    Sub(lhs, rhs, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp64> inline Mul(absl::Span<const kFp64> lhs,
                                       absl::Span<const kFp64> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    std::vector<kFp64> ret(lhs.size());
    Mul(lhs, rhs, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp64> inline ScalarMul(const kFp64 scalar,
                                             absl::Span<const kFp64> in) {
    std::vector<kFp64> ret(in.size());
    ScalarMul(scalar, in, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp64> inline Div(absl::Span<const kFp64> lhs,
                                       absl::Span<const kFp64> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    std::vector<kFp64> ret(lhs.size());
    Div(lhs, rhs, absl::MakeSpan(ret));
    return ret;
  }

  static void inline AddInplace(absl::Span<kFp64> lhs,
                                absl::Span<const kFp64> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    Add(lhs, rhs, lhs);
  }

  static void inline SubInplace(absl::Span<kFp64> lhs,
                                absl::Span<const kFp64> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    Sub(lhs, rhs, lhs);
  }

  static void inline MulInplace(absl::Span<kFp64> lhs,
                                absl::Span<const kFp64> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    Mul(lhs, rhs, lhs);
  }

  static void inline ScalarMulInplace(const kFp64& scalar,
                                      absl::Span<kFp64> in) {
    ScalarMul(scalar, in, in);
  }

  static void inline DivInplace(absl::Span<kFp64> lhs,
                                absl::Span<const kFp64> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    auto inv = Inv(rhs);
    MulInplace(lhs, absl::MakeConstSpan(inv));
  }

  static std::vector<kFp64> inline Neg(absl::Span<const kFp64> in) {
    const size_t size = in.size();
    std::vector<kFp64> ret(size);
    Neg(in, absl::MakeSpan(ret));
    return ret;
  }

  static void inline NegInplace(absl::Span<kFp64> in) { Neg(in, in); }

  static std::vector<kFp64> inline Inv(absl::Span<const kFp64> in) {
    const size_t size = in.size();
    std::vector<kFp64> ret(size);
    Inv(in, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp64> inline Ones(uint32_t num) {
    std::vector<kFp64> ret(num, kFp64::One());
    return ret;
  }

  static std::vector<kFp64> inline Zeros(uint32_t num) {
    std::vector<kFp64> ret(num, kFp64::Zero());
    return ret;
  }

  static std::vector<kFp64> inline Rand(uint32_t num) {
    std::vector<kFp64> ret(num);
    Rand(absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp64> inline Rand(yacl::crypto::Prg<uint8_t>& prg,
                                        uint32_t num) {
    std::vector<kFp64> ret(num);
    Rand(prg, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp64> inline Rand(uint128_t seed, uint32_t num) {
    auto prg = yacl::crypto::Prg<uint8_t>(seed);
    return Rand(prg, num);
  }

  // Inner product
  static kFp64 inline InPro(absl::Span<const kFp64> lhs,
                            absl::Span<const kFp64> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    const size_t size = lhs.size();
    kFp64 ret = kFp64::Zero();
    for (uint32_t i = 0; i < size; ++i) {
      ret = ret + (lhs[i] * rhs[i]);
    }
    return ret;
  }
};  // namespace vec64

class op128 {
 public:
  static void Add(absl::Span<const kFp128> lhs, absl::Span<const kFp128> rhs,
                  absl::Span<kFp128> out);

  static void Sub(absl::Span<const kFp128> lhs, absl::Span<const kFp128> rhs,
                  absl::Span<kFp128> out);

  static void Mul(absl::Span<const kFp128> lhs, absl::Span<const kFp128> rhs,
                  absl::Span<kFp128> out);

  static void ScalarMul(const kFp128 scalar, absl::Span<const kFp128> in,
                        absl::Span<kFp128> out);

  // Implement : Mul(lsh , Inv(rhs))
  static void Div(absl::Span<const kFp128> lhs, absl::Span<const kFp128> rhs,
                  absl::Span<kFp128> out);

  static void Neg(absl::Span<const kFp128> in, absl::Span<kFp128> out);

  static void Sqrt(absl::Span<const kFp128> in, absl::Span<kFp128> out);

  static void inline AddInplace(absl::Span<kFp128> lhs,
                                absl::Span<const kFp128> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    Add(lhs, rhs, lhs);
  }

  static void inline SubInplace(absl::Span<kFp128> lhs,
                                absl::Span<const kFp128> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    Sub(lhs, rhs, lhs);
  }

  static void inline MulInplace(absl::Span<kFp128> lhs,
                                absl::Span<const kFp128> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    Mul(lhs, rhs, lhs);
  }

  static void inline ScalarMulInplace(const kFp128& scalar,
                                      absl::Span<kFp128> in) {
    ScalarMul(scalar, in, in);
  }

  static void inline DivInplace(absl::Span<kFp128> lhs,
                                absl::Span<const kFp128> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    auto inv = Inv(rhs);
    MulInplace(lhs, absl::MakeConstSpan(inv));
  }

  static void inline SqrtInplace(absl::Span<kFp128> inout) {
    Sqrt(inout, inout);
  }

  // fast batch inv
  static void Inv(absl::Span<const kFp128> in, absl::Span<kFp128> out);
  static void Ones(absl::Span<kFp128> out);
  static void Zeros(absl::Span<kFp128> out);
  static void Rand(absl::Span<kFp128> out);
  static void Rand(yacl::crypto::Prg<uint8_t>& prg, absl::Span<kFp128> out);

  static std::vector<kFp128> inline Add(absl::Span<const kFp128> lhs,
                                        absl::Span<const kFp128> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    std::vector<kFp128> ret(lhs.size());
    Add(lhs, rhs, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp128> inline Sub(absl::Span<const kFp128> lhs,
                                        absl::Span<const kFp128> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    std::vector<kFp128> ret(lhs.size());
    Sub(lhs, rhs, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp128> inline Mul(absl::Span<const kFp128> lhs,
                                        absl::Span<const kFp128> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    std::vector<kFp128> ret(lhs.size());
    Mul(lhs, rhs, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp128> inline ScalarMul(const kFp128 scalar,
                                              absl::Span<const kFp128> in) {
    std::vector<kFp128> ret(in.size());
    ScalarMul(scalar, in, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp128> inline Div(absl::Span<const kFp128> lhs,
                                        absl::Span<const kFp128> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    std::vector<kFp128> ret(lhs.size());
    Div(lhs, rhs, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp128> inline Neg(absl::Span<const kFp128> in) {
    const size_t size = in.size();
    std::vector<kFp128> ret(size);
    Neg(in, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp128> inline Sqrt(absl::Span<const kFp128> in) {
    const size_t size = in.size();
    std::vector<kFp128> ret(size);
    Sqrt(in, absl::MakeSpan(ret));
    return ret;
  }

  static void inline NegInplace(absl::Span<kFp128> in) { Neg(in, in); }

  static std::vector<kFp128> inline Inv(absl::Span<const kFp128> in) {
    const size_t size = in.size();
    std::vector<kFp128> ret(size);
    Inv(in, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp128> inline Ones(uint32_t num) {
    std::vector<kFp128> ret(num, kFp128::One());
    return ret;
  }

  static std::vector<kFp128> inline Zeros(uint32_t num) {
    std::vector<kFp128> ret(num, kFp128::Zero());
    return ret;
  }

  static std::vector<kFp128> inline Rand(uint32_t num) {
    std::vector<kFp128> ret(num);
    Rand(absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp128> inline Rand(yacl::crypto::Prg<uint8_t>& prg,
                                         uint32_t num) {
    std::vector<kFp128> ret(num);
    Rand(prg, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp128> inline Rand(uint128_t seed, uint32_t num) {
    auto prg = yacl::crypto::Prg<uint8_t>(seed);
    return Rand(prg, num);
  }

  // Inner product
  static kFp128 inline InPro(absl::Span<const kFp128> lhs,
                             absl::Span<const kFp128> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    const size_t size = lhs.size();

    return yacl::parallel_reduce<
        kFp128, std::function<kFp128(uint64_t, uint64_t)>,
        std::function<kFp128(const kFp128&, const kFp128&)>>(
        0, size, 4096,
        [&](uint64_t bg, uint64_t ed) {
          kFp128 ret = kFp128::Zero();
          for (auto i = bg; i < ed; ++i) {
            ret = ret + (lhs[i] * rhs[i]);
          }
          return ret;
        },
        kFp128::Add);
  }

};  // namespace vec128

class op256 {
 public:
  static void Add(absl::Span<const kFp256> lhs, absl::Span<const kFp256> rhs,
                  absl::Span<kFp256> out);

  static void Sub(absl::Span<const kFp256> lhs, absl::Span<const kFp256> rhs,
                  absl::Span<kFp256> out);

  static void Mul(absl::Span<const kFp256> lhs, absl::Span<const kFp256> rhs,
                  absl::Span<kFp256> out);

  static void ScalarMul(const kFp256 scalar, absl::Span<const kFp256> in,
                        absl::Span<kFp256> out);

  // Implement : Mul(lsh , Inv(rhs))
  static void Div(absl::Span<const kFp256> lhs, absl::Span<const kFp256> rhs,
                  absl::Span<kFp256> out);

  static void Neg(absl::Span<const kFp256> in, absl::Span<kFp256> out);

  static void Sqrt(absl::Span<const kFp256> in, absl::Span<kFp256> out);

  static void inline AddInplace(absl::Span<kFp256> lhs,
                                absl::Span<const kFp256> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    Add(lhs, rhs, lhs);
  }

  static void inline SubInplace(absl::Span<kFp256> lhs,
                                absl::Span<const kFp256> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    Sub(lhs, rhs, lhs);
  }

  static void inline MulInplace(absl::Span<kFp256> lhs,
                                absl::Span<const kFp256> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    Mul(lhs, rhs, lhs);
  }

  static void inline ScalarMulInplace(const kFp256& scalar,
                                      absl::Span<kFp256> in) {
    ScalarMul(scalar, in, in);
  }

  static void inline DivInplace(absl::Span<kFp256> lhs,
                                absl::Span<const kFp256> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    auto inv = Inv(rhs);
    MulInplace(lhs, absl::MakeConstSpan(inv));
  }

  static void inline SqrtInplace(absl::Span<kFp256> inout) {
    Sqrt(inout, inout);
  }

  // fast batch inv
  static void Inv(absl::Span<const kFp256> in, absl::Span<kFp256> out);
  static void Ones(absl::Span<kFp256> out);
  static void Zeros(absl::Span<kFp256> out);
  static void Rand(absl::Span<kFp256> out);
  static void Rand(yacl::crypto::Prg<uint8_t>& prg, absl::Span<kFp256> out);

  static std::vector<kFp256> inline Add(absl::Span<const kFp256> lhs,
                                        absl::Span<const kFp256> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    std::vector<kFp256> ret(lhs.size());
    Add(lhs, rhs, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp256> inline Sub(absl::Span<const kFp256> lhs,
                                        absl::Span<const kFp256> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    std::vector<kFp256> ret(lhs.size());
    Sub(lhs, rhs, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp256> inline Mul(absl::Span<const kFp256> lhs,
                                        absl::Span<const kFp256> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    std::vector<kFp256> ret(lhs.size());
    Mul(lhs, rhs, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp256> inline ScalarMul(const kFp256 scalar,
                                              absl::Span<const kFp256> in) {
    std::vector<kFp256> ret(in.size());
    ScalarMul(scalar, in, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp256> inline Div(absl::Span<const kFp256> lhs,
                                        absl::Span<const kFp256> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    std::vector<kFp256> ret(lhs.size());
    Div(lhs, rhs, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp256> inline Neg(absl::Span<const kFp256> in) {
    const size_t size = in.size();
    std::vector<kFp256> ret(size);
    Neg(in, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp256> inline Sqrt(absl::Span<const kFp256> in) {
    const size_t size = in.size();
    std::vector<kFp256> ret(size);
    Sqrt(in, absl::MakeSpan(ret));
    return ret;
  }

  static void inline NegInplace(absl::Span<kFp256> in) { Neg(in, in); }

  static std::vector<kFp256> inline Inv(absl::Span<const kFp256> in) {
    const size_t size = in.size();
    std::vector<kFp256> ret(size);
    Inv(in, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp256> inline Ones(uint32_t num) {
    std::vector<kFp256> ret(num, kFp256::One());
    return ret;
  }

  static std::vector<kFp256> inline Zeros(uint32_t num) {
    std::vector<kFp256> ret(num, kFp256::Zero());
    return ret;
  }

  static std::vector<kFp256> inline Rand(uint32_t num) {
    std::vector<kFp256> ret(num);
    Rand(absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp256> inline Rand(yacl::crypto::Prg<uint8_t>& prg,
                                         uint32_t num) {
    std::vector<kFp256> ret(num);
    Rand(prg, absl::MakeSpan(ret));
    return ret;
  }

  static std::vector<kFp256> inline Rand(uint128_t seed, uint32_t num) {
    auto prg = yacl::crypto::Prg<uint8_t>(seed);
    return Rand(prg, num);
  }

  // Inner product
  static kFp256 inline InPro(absl::Span<const kFp256> lhs,
                             absl::Span<const kFp256> rhs) {
    YACL_ENFORCE(lhs.size() == rhs.size());
    const size_t size = lhs.size();

    return yacl::parallel_reduce<
        kFp256, std::function<kFp256(uint64_t, uint64_t)>,
        std::function<kFp256(const kFp256&, const kFp256&)>>(
        0, size, 4096,
        [&](uint64_t bg, uint64_t ed) {
          kFp256 ret = kFp256::Zero();
          for (auto i = bg; i < ed; ++i) {
            ret = ret + (lhs[i] * rhs[i]);
          }
          return ret;
        },
        kFp256::Add);
  }

};  // namespace vec256

// Index Shuffle
std::vector<size_t> GenPerm(uint32_t num);

inline yacl::math::MPInt ToMPInt(const uint64_t& val) {
  return yacl::math::MPInt(val);
}

inline yacl::math::MPInt ToMPInt(const uint128_t& val) {
  return yacl::math::MPInt(val);
}

inline yacl::math::MPInt ToMPInt(const uint256_t& val) {
  uint128_t high128 = uint128_t(val >> 128);
  uint128_t low128 = uint128_t(val);
  return (ToMPInt(high128) << 128) + ToMPInt(low128);
}

};  // namespace mcpsi
