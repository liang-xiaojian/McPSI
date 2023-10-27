#include "mcpsi/utils/vec_op.h"

#include "field.h"
#include "gtest/gtest.h"
namespace test {

TEST(kFp64Test, AddWork) {
  size_t num = 10000;
  auto lhs = Rand(num);
  auto rhs = Rand(num);

  auto ret = Add(absl::MakeSpan(lhs), absl::MakeSpan(rhs));
  auto ret2 = Add(absl::MakeSpan(rhs), absl::MakeSpan(lhs));

  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(ret[i], ret2[i]);
  }
}

TEST(kFp64Test, SubWork) {
  size_t num = 10000;
  auto lhs = Rand(num);
  auto rhs = Rand(num);

  auto ret = Sub(absl::MakeSpan(lhs), absl::MakeSpan(rhs));
  auto ret2 = Sub(absl::MakeSpan(rhs), absl::MakeSpan(lhs));

  auto ret3 = Add(absl::MakeSpan(ret), absl::MakeSpan(ret2));

  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(ret3[i], kFp64(0));
  }
}

TEST(kFp64Test, MulWork) {
  size_t num = 10000;
  auto lhs = Rand(num);
  auto rhs = Rand(num);

  auto ret = Mul(absl::MakeSpan(lhs), absl::MakeSpan(rhs));
  auto ret2 = Mul(absl::MakeSpan(rhs), absl::MakeSpan(lhs));

  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(ret[i], ret2[i]);
  }
}

TEST(kFp64Test, DivWork) {
  size_t num = 10000;
  auto lhs = Rand(num);
  auto rhs = Rand(num);

  auto ret = Div(absl::MakeSpan(lhs), absl::MakeSpan(rhs));
  auto ret2 = Div(absl::MakeSpan(rhs), absl::MakeSpan(lhs));

  auto ret3 = Mul(absl::MakeSpan(ret), absl::MakeSpan(ret2));

  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(ret3[i], kFp64(1));
  }
}

TEST(kFp64Test, PrgWork) {
  size_t num = 10000;
  uint128_t seed = yacl::crypto::RandU128(true);

  yacl::crypto::Prg<uint8_t> prg0(seed);
  yacl::crypto::Prg<uint8_t> prg1(seed);

  auto lhs = Rand(prg0, num);
  auto rhs = Rand(prg1, num);

  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(lhs[i], rhs[i]);
  }
}

TEST(kFp64Test, OneZeroWork) {
  size_t num = 10000;

  auto lhs = Ones(num);
  auto rhs = Ones(num);

  auto ret = Sub(absl::MakeSpan(lhs), absl::MakeSpan(rhs));
  auto zeros = Zeros(num);

  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(ret[i], zeros[i]);
  }
}

TEST(kFp64Test, ShuffleWork) {
  size_t num = 10000;

  auto p0 = GenPerm(num);
  auto p1 = GenPerm(num);

  std::sort(p0.begin(), p0.end());
  std::sort(p1.begin(), p1.end());

  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(p0[i], p1[i]);
  }
}

}  // namespace test
