#pragma once

#include "test/utils/config.h"
#include "yacl/base/int128.h"
#include "yacl/crypto/utils/rand.h"

namespace test {

namespace {
template <typename T>
T exgcd(T a, T b, T& x, T& y) {
  if (b == 0) {
    //根据上面的推理1，基本情况
    x = 1;
    y = 0;
    return a;
  }
  //根据推理2
  T r = exgcd(b, a % b, x, y);
  T t = y;
  y = x - (a / b) * y;
  x = t;
  return r;
}
}  // namespace

class kFp64 {
 public:
  kFp64() : val_(0) {}

  kFp64(int val) : val_(val % Prime64) {}

  kFp64(uint64_t val) : val_(val % Prime64) {}

  kFp64(uint128_t val) : val_(val % Prime64) {}

  kFp64 operator+(const kFp64& rhs) const { return kFp64(val_ + rhs.val_); }

  kFp64 operator-(const kFp64& rhs) const {
    return kFp64(val_ - rhs.val_ + Prime64);
  }

  kFp64 operator*(const kFp64& rhs) const {
    uint128_t l = val_;
    uint128_t r = rhs.val_;
    return kFp64(l * r);
  }

  kFp64 operator/(const kFp64& rhs) const { return (*this) * Inv(rhs); }

  bool operator==(const kFp64& rhs) const { return this->val_ == rhs.val_; }

  uint64_t GetVal() const { return val_; }

  static kFp64 Add(const kFp64& lhs, const kFp64& rhs) { return lhs + rhs; }

  static kFp64 Sub(const kFp64& lhs, const kFp64& rhs) { return lhs - rhs; }

  static kFp64 Mul(const kFp64& lhs, const kFp64& rhs) { return lhs * rhs; }

  static kFp64 Div(const kFp64& lhs, const kFp64& rhs) { return lhs / rhs; }

  static kFp64 Inv(const kFp64& in) {
    uint64_t result = 0;
    uint64_t tmp = 0;
    uint64_t check = exgcd(in.val_, Prime64, result, tmp);
    YACL_ENFORCE(check == 1);
    return kFp64(result + Prime64);
  }

  static kFp64 Neg(const kFp64& in) { return kFp64(Prime64 - in.val_); }

  static bool Equal(const kFp64& lhs, const kFp64& rhs) { return lhs == rhs; }

  static kFp64 Rand() {
    uint64_t rand_val = yacl::crypto::RandU64(true);
    return kFp64(rand_val);
  }

  static uint64_t GetPrime() { return Prime64; }

  static kFp64 One() { return kFp64(1); }

  static kFp64 Zero() { return kFp64(0); }

 protected:
  uint64_t val_;
};

};  // namespace test
