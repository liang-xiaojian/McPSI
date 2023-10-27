#include <future>

#include "gtest/gtest.h"
#include "mcpsi/context/register.h"
#include "mcpsi/ss/protocol.h"
#include "mcpsi/utils/test_util.h"
#include "mcpsi/utils/vec_op.h"

namespace test {

namespace yc = yacl::crypto;

#define CALCULATE_AP(context, num, func)     \
  auto prot = context->GetState<Protocol>(); \
  auto lhs_p = prot->RandP(num);             \
  auto rhs_p = prot->RandP(num);             \
  auto lhs_a = prot->P2A(lhs_p);             \
  auto ret_a = prot->func(lhs_a, rhs_p);     \
  auto ret = prot->A2P(ret_a);               \
  auto check = func(lhs_p, rhs_p);           \
  for (size_t i = 0; i < num; ++i) {         \
    EXPECT_EQ(check[i], ret[i]);             \
  }                                          \
  return ret;

#define DECLARE_AP_TEST(func)                                              \
  TEST(ProtoclTest, func##APWork) {                                        \
    auto context = MockContext(2);                                         \
    MockInitContext(context);                                              \
    size_t num = 10000;                                                    \
    auto rank0 = std::async([&] { CALCULATE_AP(context[0], num, func); }); \
    auto rank1 = std::async([&] { CALCULATE_AP(context[1], num, func); }); \
    auto r_b = rank0.get();                                                \
    auto r_a = rank1.get();                                                \
    for (size_t i = 0; i < num; ++i) {                                     \
      EXPECT_EQ(r_a[i], r_b[i]);                                           \
    }                                                                      \
  };

DECLARE_AP_TEST(Add);
DECLARE_AP_TEST(Sub);
DECLARE_AP_TEST(Mul);
DECLARE_AP_TEST(Div);

#define CALCULATE_PA(context, num, func)     \
  auto prot = context->GetState<Protocol>(); \
  auto lhs_p = prot->RandP(num);             \
  auto rhs_p = prot->RandP(num);             \
  auto rhs_a = prot->P2A(rhs_p);             \
  auto ret_a = prot->func(lhs_p, rhs_a);     \
  auto ret = prot->A2P(ret_a);               \
  auto check = func(lhs_p, rhs_p);           \
  for (size_t i = 0; i < num; ++i) {         \
    EXPECT_EQ(check[i], ret[i]);             \
  }                                          \
  return ret;

#define DECLARE_PA_TEST(func)                                              \
  TEST(ProtoclTest, func##PAWork) {                                        \
    auto context = MockContext(2);                                         \
    MockInitContext(context);                                              \
    size_t num = 10000;                                                    \
    auto rank0 = std::async([&] { CALCULATE_PA(context[0], num, func); }); \
    auto rank1 = std::async([&] { CALCULATE_PA(context[1], num, func); }); \
    auto r_b = rank0.get();                                                \
    auto r_a = rank1.get();                                                \
    for (size_t i = 0; i < num; ++i) {                                     \
      EXPECT_EQ(r_a[i], r_b[i]);                                           \
    }                                                                      \
  };

DECLARE_PA_TEST(Add);
DECLARE_PA_TEST(Sub);
DECLARE_PA_TEST(Mul);
DECLARE_PA_TEST(Div);

#define CALCULATE_AA(context, num, func)     \
  auto prot = context->GetState<Protocol>(); \
  auto lhs_p = prot->RandP(num);             \
  auto rhs_p = prot->RandP(num);             \
  auto lhs_a = prot->P2A(lhs_p);             \
  auto rhs_a = prot->P2A(rhs_p);             \
  auto ret_a = prot->func(lhs_a, rhs_a);     \
  auto ret = prot->A2P(ret_a);               \
  auto check = func(lhs_p, rhs_p);           \
  for (size_t i = 0; i < num; ++i) {         \
    EXPECT_EQ(check[i], ret[i]);             \
  }                                          \
  return ret;

#define DECLARE_AA_TEST(func)                                              \
  TEST(ProtoclTest, func##AAWork) {                                        \
    auto context = MockContext(2);                                         \
    MockInitContext(context);                                              \
    size_t num = 10000;                                                    \
    auto rank0 = std::async([&] { CALCULATE_AA(context[0], num, func); }); \
    auto rank1 = std::async([&] { CALCULATE_AA(context[1], num, func); }); \
    auto r_b = rank0.get();                                                \
    auto r_a = rank1.get();                                                \
    for (size_t i = 0; i < num; ++i) {                                     \
      EXPECT_EQ(r_a[i], r_b[i]);                                           \
    }                                                                      \
  };

DECLARE_AA_TEST(Add);
DECLARE_AA_TEST(Sub);
DECLARE_AA_TEST(Mul);
DECLARE_AA_TEST(Div);

TEST(ProtoclTest, ZeroTest) {
  auto context = MockContext(2);
  MockInitContext(context);
  size_t num = 10000;
  auto rank0 = std::async([&] {
    auto prot = context[0]->GetState<Protocol>();
    auto lhs = prot->ZerosA(num);
    auto ret = prot->A2P(lhs);
    return ret;
  });
  auto rank1 = std::async([&] {
    auto prot = context[1]->GetState<Protocol>();
    auto lhs = prot->ZerosA(num);
    auto ret = prot->A2P(lhs);
    return ret;
  });
  auto r_b = rank0.get();
  auto r_a = rank1.get();
  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(r_a[i], r_b[i]);
    EXPECT_EQ(r_a[i], kFp64::Zero());
  }
};

TEST(ProtoclTest, RandATest) {
  auto context = MockContext(2);
  MockInitContext(context);
  size_t num = 10000;
  auto rank0 = std::async([&] {
    auto prot = context[0]->GetState<Protocol>();
    auto lhs = prot->RandA(num);
    auto ret = prot->A2P(lhs);
    return ret;
  });
  auto rank1 = std::async([&] {
    auto prot = context[1]->GetState<Protocol>();
    auto lhs = prot->RandA(num);
    auto ret = prot->A2P(lhs);
    return ret;
  });
  auto r_b = rank0.get();
  auto r_a = rank1.get();
  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(r_a[i], r_b[i]);
  }
};

TEST(ProtoclTest, InvATest) {
  auto context = MockContext(2);
  MockInitContext(context);
  size_t num = 10000;
  auto rank0 = std::async([&] {
    auto prot = context[0]->GetState<Protocol>();
    auto lhs = prot->RandA(num);
    auto rhs = prot->Inv(lhs);
    auto ret_a = prot->Mul(lhs, rhs);
    auto ret_p = prot->A2P(ret_a);
    return ret_p;
  });
  auto rank1 = std::async([&] {
    auto prot = context[1]->GetState<Protocol>();
    auto lhs = prot->RandA(num);
    auto rhs = prot->Inv(lhs);
    auto ret_a = prot->Mul(lhs, rhs);
    auto ret_p = prot->A2P(ret_a);
    return ret_p;
  });
  auto r_b = rank0.get();
  auto r_a = rank1.get();
  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(r_a[i], r_b[i]);
    EXPECT_EQ(r_a[i], kFp64::One());
  }
};

TEST(ProtoclTest, ConvertTest) {
  auto context = MockContext(2);
  MockInitContext(context);
  size_t num = 10000;
  auto rank0 = std::async([&] {
    auto prot = context[0]->GetState<Protocol>();
    auto lhs = prot->RandP(num);
    auto a = prot->P2A(lhs);
    auto ret = prot->A2P(a);
    return ret;
  });
  auto rank1 = std::async([&] {
    auto prot = context[1]->GetState<Protocol>();
    auto lhs = prot->RandP(num);
    auto a = prot->P2A(lhs);
    auto ret = prot->A2P(a);
    return ret;
  });
  auto r_b = rank0.get();
  auto r_a = rank1.get();
  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(r_a[i], r_b[i]);
  }
};

// Shuffle (one-side) Test
TEST(ProtoclTest, ShuffleOneSideTest) {
  auto context = MockContext(2);
  MockInitContext(context);
  size_t num = 10000;
  auto rank0 = std::async([&] {
    auto prot = context[0]->GetState<Protocol>();
    auto r_p = prot->RandP(num);
    auto r_a = prot->P2A(r_p);
    auto perm = GenPerm(num);
    auto s_a = prot->ShuffleASet(r_a, perm);
    auto s_p = prot->A2P(s_a);
    for (size_t i = 0; i < num; ++i) {
      EXPECT_EQ(s_p[i], r_p[perm[i]]);
    }
    return s_p;
  });
  auto rank1 = std::async([&] {
    auto prot = context[1]->GetState<Protocol>();
    auto r_p = prot->RandP(num);
    auto r_a = prot->P2A(r_p);
    auto s_a = prot->ShuffleAGet(r_a);
    auto s_p = prot->A2P(s_a);
    return s_p;
  });
  rank0.get();
  rank1.get();
};

// Shuffle (two side) Test
TEST(ProtoclTest, ShuffleTwoSideTest) {
  auto context = MockContext(2);
  MockInitContext(context);
  size_t num = 10000;
  auto rank0 = std::async([&] {
    auto prot = context[0]->GetState<Protocol>();
    auto r_p = prot->RandP(num);
    auto r_a = prot->P2A(r_p);
    auto s_a = prot->ShuffleA(r_a);
    auto s_p = prot->A2P(s_a);

    std::vector<uint64_t> sort_r(num);
    std::vector<uint64_t> sort_s(num);
    memcpy(sort_r.data(), r_p.data(), num * sizeof(internal::PTy));
    memcpy(sort_s.data(), s_p.data(), num * sizeof(internal::PTy));

    std::sort(sort_r.begin(), sort_r.end());
    std::sort(sort_s.begin(), sort_s.end());

    for (size_t i = 0; i < num; ++i) {
      EXPECT_EQ(sort_r[i], sort_s[i]);
    }
    return 0;
  });
  auto rank1 = std::async([&] {
    auto prot = context[1]->GetState<Protocol>();
    auto r_p = prot->RandP(num);
    auto r_a = prot->P2A(r_p);
    auto s_a = prot->ShuffleA(r_a);
    auto s_p = prot->A2P(s_a);

    std::vector<uint64_t> sort_r(num);
    std::vector<uint64_t> sort_s(num);
    memcpy(sort_r.data(), r_p.data(), num * sizeof(internal::PTy));
    memcpy(sort_s.data(), s_p.data(), num * sizeof(internal::PTy));

    std::sort(sort_r.begin(), sort_r.end());
    std::sort(sort_s.begin(), sort_s.end());

    for (size_t i = 0; i < num; ++i) {
      EXPECT_EQ(sort_r[i], sort_s[i]);
    }
    return 0;
  });
  rank0.get();
  rank1.get();
};

TEST(ProtoclTest, SetATest) {
  auto context = MockContext(2);
  MockInitContext(context);
  size_t num = 10000;
  auto rank0 = std::async([&] {
    auto prot = context[0]->GetState<Protocol>();
    auto rand = Rand(num);
    auto ret_a = prot->SetA(rand);
    [[maybe_unused]] auto ret_p = prot->A2P(ret_a);
    return rand;
  });
  auto rank1 = std::async([&] {
    auto prot = context[1]->GetState<Protocol>();
    auto ret_a = prot->GetA(num);
    auto ret_p = prot->A2P(ret_a);
    return ret_p;
  });
  auto r_b = rank0.get();
  auto r_a = rank1.get();
  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(r_a[i], r_b[i]);
  }
}

};  // namespace test
