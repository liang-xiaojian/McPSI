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
    MockSetupContext(ctx);
    return ctx;
  }
};

std::vector<std::shared_ptr<Context>> TestParam::ctx =
    std::vector<std::shared_ptr<Context>>();

TEST(Setup, InitializeWork) {
  auto context = TestParam::GetContext();
  EXPECT_EQ(context.size(), 2);
}

TEST(ProtocolTest, DyOprfTest) {
  auto context = TestParam::GetContext();
  size_t num = 10000;
  auto rank0 = std::async([&] {
    auto prot = context[0]->GetState<Protocol>();
    auto r_p = prot->RandP(num);
    auto lhs = prot->P2A(r_p);
    auto rhs = prot->P2A(r_p);
    auto ret0 = prot->DyOprf(lhs);
    auto ret1 = prot->DyOprf(rhs);
    return context[0]->GetRank() == 0 ? ret0 : ret1;
  });
  auto rank1 = std::async([&] {
    auto prot = context[1]->GetState<Protocol>();
    auto r_p = prot->RandP(num);
    auto lhs = prot->P2A(r_p);
    auto rhs = prot->P2A(r_p);
    auto ret0 = prot->DyOprf(lhs);
    auto ret1 = prot->DyOprf(rhs);
    return context[1]->GetRank() == 0 ? ret0 : ret1;
  });
  auto r_a = rank0.get();
  auto r_b = rank1.get();

  // auto group = yc::EcGroupFactory::Instance().Create("secp128r2",
  //                                                    yacl::ArgLib =
  //                                                    "openssl");
  auto group = yc::EcGroupFactory::Instance().Create(
      internal::kCurveName, yacl::ArgLib = internal::kCurveLib);
  for (size_t i = 0; i < num; ++i) {
    EXPECT_TRUE(group->PointEqual(r_a[i], r_b[i]));
  }
};

TEST(ProtocolTest, ScalarDyOprfTest) {
  auto context = TestParam::GetContext();
  size_t num = 10000;
  auto r_p = internal::op::Rand(num + 1);
  auto rank0 = std::async([&] {
    auto prot = context[0]->GetState<Protocol>();
    auto r_a = prot->P2A(r_p);

    auto ret0 = prot->DyOprf(absl::MakeSpan(r_a).subspan(0, num));
    auto ret1 =
        prot->ScalarDyOprf(r_a[num], absl::MakeSpan(r_a).subspan(0, num));
    return context[0]->GetRank() == 0 ? ret0 : ret1;
  });
  auto rank1 = std::async([&] {
    auto prot = context[1]->GetState<Protocol>();
    auto r_a = prot->P2A(r_p);

    auto ret0 = prot->DyOprf(absl::MakeSpan(r_a).subspan(0, num));
    auto ret1 =
        prot->ScalarDyOprf(r_a[num], absl::MakeSpan(r_a).subspan(0, num));
    return context[1]->GetRank() == 0 ? ret0 : ret1;
  });
  auto ret0 = rank0.get();
  auto ret1 = rank1.get();

  // auto group = yc::EcGroupFactory::Instance().Create("secp128r2",
  //                                                    yacl::ArgLib =
  //                                                    "openssl");
  auto group = yc::EcGroupFactory::Instance().Create(
      internal::kCurveName, yacl::ArgLib = internal::kCurveLib);
  auto scalar = ym::MPInt(r_p[num].GetVal());
  for (size_t i = 0; i < num; ++i) {
    EXPECT_TRUE(group->PointEqual(group->Mul(ret0[i], scalar), ret1[i]));
  }
};

}  // namespace mcpsi