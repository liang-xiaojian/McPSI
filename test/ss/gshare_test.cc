#include <future>

#include "gtest/gtest.h"
#include "test/context/register.h"
#include "test/ss/protocol.h"
#include "test/utils/test_util.h"
#include "test/utils/vec_op.h"

namespace test {

namespace yc = yacl::crypto;

TEST(ProtoclTest, A2GTest) {
  auto context = MockContext(2);
  MockInitContext(context);
  size_t num = 10000;
  auto rank0 = std::async([&] {
    auto prot = context[0]->GetState<Protocol>();
    auto r_p = prot->RandP(num);
    auto lhs = prot->P2A(r_p);
    auto rhs = prot->P2A(r_p);
    auto ret0 = prot->A2G(lhs);
    auto ret1 = prot->A2G(rhs);
    return context[0]->GetRank() == 0 ? ret0 : ret1;
  });
  auto rank1 = std::async([&] {
    auto prot = context[1]->GetState<Protocol>();
    auto r_p = prot->RandP(num);
    auto lhs = prot->P2A(r_p);
    auto rhs = prot->P2A(r_p);
    auto ret0 = prot->A2G(lhs);
    auto ret1 = prot->A2G(rhs);
    return context[1]->GetRank() == 0 ? ret0 : ret1;
  });
  auto r_b = rank0.get();
  auto r_a = rank1.get();
  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(r_a[i], r_b[i]);
  }
};

}  // namespace test