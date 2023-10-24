#include <future>

#include "gtest/gtest.h"
#include "test/context/register.h"
#include "test/ss/protocol.h"
#include "test/utils/test_util.h"

namespace test {

namespace yc = yacl::crypto;

#define CALCULATE(context, num, func)        \
  auto prot = context->GetState<Protocol>(); \
  auto lhs = prot->RandP(num);               \
  auto rhs = prot->RandP(num);               \
  return prot->func(lhs, rhs);

#define DECLARE_PP_TEST(func)                                           \
  TEST(ProtoclTest, func##Work) {                                       \
    auto context = MockContext(2);                                      \
    MockInitContext(context);                                           \
    size_t num = 10000;                                                 \
    auto rank0 = std::async([&] { CALCULATE(context[0], num, func); }); \
    auto rank1 = std::async([&] { CALCULATE(context[1], num, func); }); \
    auto r_b = rank0.get();                                             \
    auto r_a = rank1.get();                                             \
    for (size_t i = 0; i < num; ++i) {                                  \
      EXPECT_EQ(r_a[i], r_b[i]);                                        \
    }                                                                   \
  };

DECLARE_PP_TEST(Add);
DECLARE_PP_TEST(Sub);
DECLARE_PP_TEST(Mul);
DECLARE_PP_TEST(Div);

};  // namespace test
