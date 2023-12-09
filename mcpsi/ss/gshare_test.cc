#include <future>

#include "gtest/gtest.h"
#include "mcpsi/context/register.h"
#include "mcpsi/ss/protocol.h"
#include "mcpsi/utils/test_util.h"
#include "mcpsi/utils/vec_op.h"

namespace mcpsi {

namespace yc = yacl::crypto;

class TestParam {
 public:
  static std::vector<std::shared_ptr<Context>> ctx;

  // Getter
  static std::vector<std::shared_ptr<Context>>& GetContext() {
    if (ctx.empty()) {
      ctx = Setup();
    }
    return ctx;
  }

  static std::vector<std::shared_ptr<Context>> Setup() {
    auto ctx = MockContext(2);
    MockInitContext(ctx);
    return ctx;
  }
};

std::vector<std::shared_ptr<Context>> TestParam::ctx =
    std::vector<std::shared_ptr<Context>>();

TEST(Setup, InitializeWork) {
  auto context = TestParam::GetContext();
  EXPECT_EQ(context.size(), 2);
}

TEST(ProtocolTest, A2GTest) {
  auto context = TestParam::GetContext();
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

}  // namespace mcpsi