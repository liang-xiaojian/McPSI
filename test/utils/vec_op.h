#pragma once
#include <algorithm>
#include <random>
#include <vector>

#include "field.h"
#include "test/utils/field.h"
#include "yacl/crypto/tools/prg.h"

namespace test {

void Add(absl::Span<const kFp64> lhs, absl::Span<const kFp64> rhs,
         absl::Span<kFp64> out);

void Sub(absl::Span<const kFp64> lhs, absl::Span<const kFp64> rhs,
         absl::Span<kFp64> out);

void Mul(absl::Span<const kFp64> lhs, absl::Span<const kFp64> rhs,
         absl::Span<kFp64> out);

void ScalarMul(const kFp64 scalar, absl::Span<const kFp64> in,
               absl::Span<kFp64> out);

void Div(absl::Span<const kFp64> lhs, absl::Span<const kFp64> rhs,
         absl::Span<kFp64> out);

void Neg(absl::Span<const kFp64> in, absl::Span<kFp64> out);

void Inv(absl::Span<const kFp64> in, absl::Span<kFp64> out);

void Ones(absl::Span<kFp64> out);

void Zeros(absl::Span<kFp64> out);

void Rand(absl::Span<kFp64> out);

void Rand(yacl::crypto::Prg<uint8_t>& prg, absl::Span<kFp64> out);

std::vector<kFp64> inline Add(absl::Span<const kFp64> lhs,
                              absl::Span<const kFp64> rhs) {
  YACL_ENFORCE(lhs.size() == rhs.size());
  std::vector<kFp64> ret(lhs.size());
  Add(lhs, rhs, absl::MakeSpan(ret));
  return ret;
}

std::vector<kFp64> inline Sub(absl::Span<const kFp64> lhs,
                              absl::Span<const kFp64> rhs) {
  YACL_ENFORCE(lhs.size() == rhs.size());
  std::vector<kFp64> ret(lhs.size());
  Sub(lhs, rhs, absl::MakeSpan(ret));
  return ret;
}

std::vector<kFp64> inline Mul(absl::Span<const kFp64> lhs,
                              absl::Span<const kFp64> rhs) {
  YACL_ENFORCE(lhs.size() == rhs.size());
  std::vector<kFp64> ret(lhs.size());
  Mul(lhs, rhs, absl::MakeSpan(ret));
  return ret;
}

std::vector<kFp64> inline ScalarMul(const kFp64 scalar,
                                    absl::Span<const kFp64> in) {
  std::vector<kFp64> ret(in.size());
  ScalarMul(scalar, in, absl::MakeSpan(ret));
  return ret;
}

std::vector<kFp64> inline Div(absl::Span<const kFp64> lhs,
                              absl::Span<const kFp64> rhs) {
  YACL_ENFORCE(lhs.size() == rhs.size());
  std::vector<kFp64> ret(lhs.size());
  Div(lhs, rhs, absl::MakeSpan(ret));
  return ret;
}

std::vector<kFp64> inline Neg(absl::Span<const kFp64> in) {
  const size_t size = in.size();
  std::vector<kFp64> ret(size);
  Neg(in, absl::MakeSpan(ret));
  return ret;
}

std::vector<kFp64> inline Inv(absl::Span<const kFp64> in) {
  const size_t size = in.size();
  std::vector<kFp64> ret(size);
  Inv(in, absl::MakeSpan(ret));
  return ret;
}

std::vector<kFp64> inline Ones(uint32_t num) {
  std::vector<kFp64> ret(num);
  Ones(absl::MakeSpan(ret));
  return ret;
}

std::vector<kFp64> inline Zeros(uint32_t num) {
  std::vector<kFp64> ret(num);
  Zeros(absl::MakeSpan(ret));
  return ret;
}

std::vector<kFp64> inline Rand(uint32_t num) {
  std::vector<kFp64> ret(num);
  Rand(absl::MakeSpan(ret));
  return ret;
}

std::vector<kFp64> inline Rand(yacl::crypto::Prg<uint8_t>& prg, uint32_t num) {
  std::vector<kFp64> ret(num);
  Rand(prg, absl::MakeSpan(ret));
  return ret;
}

// Inner product
kFp64 inline InPro(absl::Span<const kFp64> lhs, absl::Span<const kFp64> rhs) {
  YACL_ENFORCE(lhs.size() == rhs.size());
  const size_t size = lhs.size();
  kFp64 ret = kFp64::Zero();
  for (uint32_t i = 0; i < size; ++i) {
    ret = ret + (lhs[i] * rhs[i]);
  }
  return ret;
}

// Index Shuffle
std::vector<size_t> GenPerm(uint32_t num);

};  // namespace test
