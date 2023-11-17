#include "mcpsi/cr/cr.h"

#include <future>

#include "gtest/gtest.h"
#include "mcpsi/context/register.h"
#include "mcpsi/utils/test_util.h"
#include "mcpsi/utils/vec_op.h"

namespace mcpsi {

TEST(CrTest, SetValueWork) {
  auto context = MockContext(2);
  MockInitContext(context);
  const size_t num = 10000;

  auto value = Rand(num);

  auto rank0 = std::async([&] {
    auto cr = context[0]->GetState<Correlation>();
    std::vector<internal::ATy> out(num, {0, 0});
    cr->AuthSet(absl::MakeConstSpan(value), absl::MakeSpan(out));
    return out;
  });
  auto rank1 = std::async([&] {
    auto cr = context[1]->GetState<Correlation>();
    std::vector<internal::ATy> out(num, {0, 0});
    cr->AuthGet(absl::MakeSpan(out));
    return out;
  });

  auto ret0 = rank0.get();
  auto ret1 = rank1.get();

  auto mac0 = internal::ExtractMac(absl::MakeConstSpan(ret0));
  auto mac1 = internal::ExtractMac(absl::MakeConstSpan(ret1));
  auto mac = Add(absl::MakeConstSpan(mac0), absl::MakeConstSpan(mac1));
  auto check = ScalarMul(context[0]->GetState<Correlation>()->GetKey() +
                             context[1]->GetState<Correlation>()->GetKey(),
                         absl::MakeSpan(value));
  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(check[i], mac[i]);
  }
}

TEST(CrTest, AuthBeaverWork) {
  auto context = MockContext(2);
  MockInitContext(context);
  const size_t num = 10000;

  auto rank0 = std::async([&] {
    auto cr = context[0]->GetState<Correlation>();
    std::vector<internal::ATy> a(num, {0, 0});
    std::vector<internal::ATy> b(num, {0, 0});
    std::vector<internal::ATy> c(num, {0, 0});
    cr->BeaverTriple(absl::MakeSpan(a), absl::MakeSpan(b), absl::MakeSpan(c));
    return std::make_tuple(a, b, c);
  });
  auto rank1 = std::async([&] {
    auto cr = context[1]->GetState<Correlation>();
    std::vector<internal::ATy> a(num, {0, 0});
    std::vector<internal::ATy> b(num, {0, 0});
    std::vector<internal::ATy> c(num, {0, 0});
    cr->BeaverTriple(absl::MakeSpan(a), absl::MakeSpan(b), absl::MakeSpan(c));
    return std::make_tuple(a, b, c);
  });

  auto [a0, b0, c0] = rank0.get();
  auto [a1, b1, c1] = rank1.get();

  auto a0_val = internal::ExtractVal(a0);
  auto a1_val = internal::ExtractVal(a1);
  auto b0_val = internal::ExtractVal(b0);
  auto b1_val = internal::ExtractVal(b1);
  auto c0_val = internal::ExtractVal(c0);
  auto c1_val = internal::ExtractVal(c1);

  auto a = Add(absl::MakeSpan(a0_val), absl::MakeSpan(a1_val));
  auto b = Add(absl::MakeSpan(b0_val), absl::MakeSpan(b1_val));
  auto c = Add(absl::MakeSpan(c0_val), absl::MakeSpan(c1_val));

  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(a[i] * b[i], c[i]);
  }
}

}  // namespace mcpsi