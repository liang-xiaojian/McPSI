#pragma once

// #include "gmp.h"
#include "boost/multiprecision/cpp_int.hpp"
#include "mcpsi/utils/config.h"
#include "yacl/base/int128.h"
#include "yacl/crypto/utils/rand.h"

namespace mcpsi {

// using uint256_t = boost::multiprecision::uint256_t;
using uint512_t = boost::multiprecision::uint512_t;

namespace {

uint64_t inline exgcd64(uint64_t a, uint64_t b, uint64_t &x, uint64_t &y) {
  uint64_t x1 = 1, x2 = 0, x3 = 0, x4 = 1;
  while (b != 0) {
    uint64_t c = a / b;
    std::tie(x1, x2, x3, x4, a, b) =
        std::make_tuple(x3, x4, x1 - x3 * c, x2 - x4 * c, b, a - b * c);
  }
  x = x1, y = x2;
  return a;
}

uint128_t inline exgcd128(uint128_t a, uint128_t b, uint128_t &x,
                          uint128_t &y) {
  uint128_t x1 = 1, x2 = 0, x3 = 0, x4 = 1;
  while (b != static_cast<uint128_t>(0)) {
    uint128_t c = a / b;
    std::tie(x1, x2, x3, x4, a, b) =
        std::make_tuple(x3, x4, x1 - x3 * c, x2 - x4 * c, b, a - b * c);
  }
  x = x1, y = x2;
  return a;
}

uint256_t inline exgcd256(uint256_t a, uint256_t b, uint256_t &x,
                          uint256_t &y) {
  uint256_t x1 = 1, x2 = 0, x3 = 0, x4 = 1;
  while (b != static_cast<uint256_t>(0)) {
    uint256_t c = a / b;
    std::tie(x1, x2, x3, x4, a, b) =
        std::make_tuple(x3, x4, x1 - x3 * c, x2 - x4 * c, b, a - b * c);
  }
  x = x1, y = x2;
  return a;
}

// Invert function in GMP is not as efficient as repect
// 10000 times "Div" ( GMP 27ms v.s. exgcd 12ms )

// uint64_t inline gmp_invert(uint64_t in) {
//   mpz_t tmp;
//   mpz_init(tmp);
//   mpz_set_ui(tmp, in);
//   mpz_t ret;
//   mpz_init(ret);
//   mpz_invert(ret, tmp, GMP_Prime64);
//   return mpz_get_ui(ret);
// }

}  // namespace

inline uint512_t ToUint512(const uint128_t &val128) {
  return uint512_t(val128);
}

inline uint512_t ToUint512(const uint256_t &val256) {
  return ToUint512(uint128_t(val256)) +
         (ToUint512(uint128_t(val256 >> 128)) << 128);
}

inline uint256_t ToUint256(const uint512_t &val512) {
  auto low128 = static_cast<uint128_t>(val512);
  auto high128 = static_cast<uint128_t>(val512 >> 128);
  return uint256_t(high128, low128);
}

class kFp64 {
 public:
  kFp64() : val_(0) {}

  kFp64(int val) : val_(val % Prime64) {}

  kFp64(uint64_t val) : val_(val % Prime64) {}

  kFp64(uint128_t val) : val_(val % Prime64) {}

  kFp64 operator+(const kFp64 &rhs) const { return kFp64(val_ + rhs.val_); }

  kFp64 operator-(const kFp64 &rhs) const {
    return kFp64(val_ - rhs.val_ + Prime64);
  }

  kFp64 operator*(const kFp64 &rhs) const {
    uint128_t l = val_;
    uint128_t r = rhs.val_;
    return kFp64(l * r);
  }

  kFp64 operator/(const kFp64 &rhs) const { return (*this) * Inv(rhs); }

  bool operator==(const kFp64 &rhs) const { return this->val_ == rhs.val_; }

  bool operator!=(const kFp64 &rhs) const { return !(*this == rhs); }

  uint64_t GetVal() const { return val_; }

  static kFp64 Add(const kFp64 &lhs, const kFp64 &rhs) { return lhs + rhs; }

  static kFp64 Sub(const kFp64 &lhs, const kFp64 &rhs) { return lhs - rhs; }

  static kFp64 Mul(const kFp64 &lhs, const kFp64 &rhs) { return lhs * rhs; }

  static kFp64 Div(const kFp64 &lhs, const kFp64 &rhs) { return lhs / rhs; }

  static kFp64 Inv(const kFp64 &in) {
    // uint64_t result = gmp_invert(in.val_);
    uint64_t result = 0, _ = 0;
    uint64_t check = exgcd64(in.val_, Prime64, result, _);
    YACL_ENFORCE(check == 1);
    return kFp64(result + Prime64);
  }

  static kFp64 Neg(const kFp64 &in) { return kFp64(Prime64 - in.val_); }

  static bool Equal(const kFp64 &lhs, const kFp64 &rhs) { return lhs == rhs; }

  static kFp64 Rand() {
    uint64_t rand_val = yacl::crypto::SecureRandU64();
    return kFp64(rand_val);
  }

  static uint64_t GetPrime() { return Prime64; }

  static kFp64 One() { return kFp64(1); }

  static kFp64 Zero() { return kFp64(0); }

 protected:
  uint64_t val_;
};

class kFp128 {
 public:
  kFp128() : val_(yacl::MakeUint128(0, 0)) {}

  kFp128(int val) : kFp128(val + Prime128) {}

  kFp128(uint64_t val) : val_(yacl::MakeUint128(0, val)) {}

  kFp128(uint128_t val) : val_(val % Prime128) {}

  kFp128(uint256_t val) : val_(val % Prime128) {}

  kFp128 operator+(const kFp128 &rhs) const { return kFp128(val_ + rhs.val_); }

  kFp128 operator-(const kFp128 &rhs) const {
    return kFp128(val_ + Prime128 - rhs.val_);
  }

  kFp128 operator*(const kFp128 &rhs) const {
    uint256_t l = val_;
    uint256_t r = rhs.val_;
    return kFp128(l * r);
  }

  kFp128 operator/(const kFp128 &rhs) const { return (*this) * Inv(rhs); }

  bool operator==(const kFp128 &rhs) const { return this->val_ == rhs.val_; }

  bool operator!=(const kFp128 &rhs) const { return !(*this == rhs); }

  uint128_t GetVal() const { return val_; }

  static kFp128 Add(const kFp128 &lhs, const kFp128 &rhs) { return lhs + rhs; }

  static kFp128 Sub(const kFp128 &lhs, const kFp128 &rhs) { return lhs - rhs; }

  static kFp128 Mul(const kFp128 &lhs, const kFp128 &rhs) { return lhs * rhs; }

  static kFp128 Div(const kFp128 &lhs, const kFp128 &rhs) { return lhs / rhs; }

  static kFp128 Inv(const kFp128 &in) {
    // uint64_t result = gmp_invert(in.val_);
    uint128_t result = 0, _ = 0;
    uint128_t check = exgcd128(in.val_, Prime128, result, _);
    YACL_ENFORCE(check == 1, "current check is {}", check);
    return kFp128(result + Prime128);
  }

  static kFp128 Neg(const kFp128 &in) { return kFp128(Prime128 - in.val_); }

  static bool Equal(const kFp128 &lhs, const kFp128 &rhs) { return lhs == rhs; }

  static kFp128 Rand() {
    uint128_t rand_val = yacl::crypto::SecureRandU128();
    return kFp128(rand_val);
  }

  static uint128_t GetPrime() { return Prime128; }

  static kFp128 One() { return kFp128(1); }

  static kFp128 Zero() { return kFp128(0); }

 protected:
  uint128_t val_;
};

class kFp256 {
 public:
  kFp256() : val_(yacl::MakeUint128(0, 0)) {}

  kFp256(int val) : val_(val) {}

  kFp256(uint64_t val) : val_(yacl::MakeUint128(0, val)) {}

  kFp256(uint128_t val) : val_(val) {}

  kFp256(uint256_t val) : val_(val % Prime256) {}

  kFp256(uint512_t val) {
    uint512_t _prime = ToUint512(Prime256);
    uint512_t val512 = val % _prime;
    val_ = ToUint256(val512);
  }

  kFp256 operator+(const kFp256 &rhs) const { return kFp256(val_ + rhs.val_); }

  kFp256 operator-(const kFp256 &rhs) const {
    return kFp256(val_ + Prime256 - rhs.val_);
  }

  kFp256 operator*(const kFp256 &rhs) const {
    auto l = ToUint512(val_);
    auto r = ToUint512(rhs.val_);
    return kFp256(l * r);
  }

  kFp256 operator/(const kFp256 &rhs) const { return (*this) * Inv(rhs); }

  bool operator==(const kFp256 &rhs) const { return this->val_ == rhs.val_; }

  bool operator!=(const kFp256 &rhs) const { return !(*this == rhs); }

  uint256_t GetVal() const { return val_; }

  static kFp256 Add(const kFp256 &lhs, const kFp256 &rhs) { return lhs + rhs; }

  static kFp256 Sub(const kFp256 &lhs, const kFp256 &rhs) { return lhs - rhs; }

  static kFp256 Mul(const kFp256 &lhs, const kFp256 &rhs) { return lhs * rhs; }

  static kFp256 Div(const kFp256 &lhs, const kFp256 &rhs) { return lhs / rhs; }

  static kFp256 Inv(const kFp256 &in) {
    // uint64_t result = gmp_invert(in.val_);
    uint256_t result = 0, _ = 0;
    uint256_t check = exgcd256(in.val_, Prime256, result, _);
    YACL_ENFORCE(check == 1, "current check is {}", check);
    return kFp256(result + Prime256);
  }

  static kFp256 Neg(const kFp256 &in) { return kFp256(Prime256 - in.val_); }

  static bool Equal(const kFp256 &lhs, const kFp256 &rhs) { return lhs == rhs; }

  static kFp256 Rand() {
    uint256_t rand_val;
    yacl::crypto::FillRand(reinterpret_cast<char *>(&rand_val), 32);
    return kFp256(rand_val);
  }

  static uint256_t GetPrime() { return Prime256; }

  static kFp256 One() { return kFp256(1); }

  static kFp256 Zero() { return kFp256(0); }

 protected:
  uint256_t val_;
};

};  // namespace mcpsi
