#include "mcpsi/cr/cr.h"

#include <future>

#include "gtest/gtest.h"
#include "mcpsi/context/register.h"
#include "mcpsi/utils/test_util.h"
#include "mcpsi/utils/vec_op.h"

namespace mcpsi {

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

TEST(CrTest, AuthBeaverWork) {
  auto context = TestParam::GetContext();
  const size_t num = 1000;

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

  auto a = internal::op::Add(absl::MakeSpan(a0_val), absl::MakeSpan(a1_val));
  auto b = internal::op::Add(absl::MakeSpan(b0_val), absl::MakeSpan(b1_val));
  auto c = internal::op::Add(absl::MakeSpan(c0_val), absl::MakeSpan(c1_val));

  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(a[i] * b[i], c[i]);
  }
}

TEST(CrTest, AuthDyBeaverWork) {
  auto context = TestParam::GetContext();
  const size_t num = 1000;

  auto rank0 = std::async([&] {
    auto cr = context[0]->GetState<Correlation>();
    std::vector<internal::ATy> a(num, {0, 0});
    std::vector<internal::ATy> b(num, {0, 0});
    std::vector<internal::ATy> c(num, {0, 0});
    std::vector<internal::ATy> r(num, {0, 0});
    cr->DyBeaverTripleSet(absl::MakeSpan(a), absl::MakeSpan(b),
                          absl::MakeSpan(c), absl::MakeSpan(r));
    internal::ATy dy_key = cr->GetDyKey();
    return std::make_tuple(a, b, c, r, std::vector<internal::ATy>(1, dy_key));
  });
  auto rank1 = std::async([&] {
    auto cr = context[1]->GetState<Correlation>();
    std::vector<internal::ATy> a(num, {0, 0});
    std::vector<internal::ATy> b(num, {0, 0});
    std::vector<internal::ATy> c(num, {0, 0});
    std::vector<internal::ATy> r(num, {0, 0});
    cr->DyBeaverTripleGet(absl::MakeSpan(a), absl::MakeSpan(b),
                          absl::MakeSpan(c), absl::MakeSpan(r));
    internal::ATy dy_key = cr->GetDyKey();
    return std::make_tuple(a, b, c, r, std::vector<internal::ATy>(1, dy_key));
  });

  auto [a0, b0, c0, r0, k0] = rank0.get();
  auto [a1, b1, c1, r1, k1] = rank1.get();
  auto a0_val = internal::ExtractVal(a0);
  auto a1_val = internal::ExtractVal(a1);
  auto b0_val = internal::ExtractVal(b0);
  auto b1_val = internal::ExtractVal(b1);
  auto c0_val = internal::ExtractVal(c0);
  auto c1_val = internal::ExtractVal(c1);
  auto r0_val = internal::ExtractVal(r0);
  auto r1_val = internal::ExtractVal(r1);

  auto a = internal::op::Add(absl::MakeSpan(a0_val), absl::MakeSpan(a1_val));
  auto b = internal::op::Add(absl::MakeSpan(b0_val), absl::MakeSpan(b1_val));
  auto c = internal::op::Add(absl::MakeSpan(c0_val), absl::MakeSpan(c1_val));
  auto r = internal::op::Add(absl::MakeSpan(r0_val), absl::MakeSpan(r1_val));

  auto k = k0[0].val + k1[0].val;

  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(b[i], b0_val[i]);
    EXPECT_EQ(a[i] * b[i], c[i]);
    EXPECT_EQ(a[i] * k, r[i]);
  }
}

TEST(CrTest, AuthBeaverCacheWork) {
  auto context = TestParam::GetContext();
  const size_t num = 1000;

  auto rank0 = std::async([&] {
    auto cr = context[0]->GetState<Correlation>();
    cr->force_cache(num, 0, 0, 0, 0);
    auto [a, b, c] = cr->BeaverTriple(num);
    return std::make_tuple(a, b, c);
  });
  auto rank1 = std::async([&] {
    auto cr = context[1]->GetState<Correlation>();
    cr->force_cache(num, 0, 0, 0, 0);
    auto [a, b, c] = cr->BeaverTriple(num);
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

  auto a = internal::op::Add(absl::MakeSpan(a0_val), absl::MakeSpan(a1_val));
  auto b = internal::op::Add(absl::MakeSpan(b0_val), absl::MakeSpan(b1_val));
  auto c = internal::op::Add(absl::MakeSpan(c0_val), absl::MakeSpan(c1_val));

  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(a[i] * b[i], c[i]);
  }
}

TEST(CrTest, AuthDyBeaverCacheWork) {
  auto context = TestParam::GetContext();
  const size_t num = 1000;

  auto rank0 = std::async([&] {
    auto cr = context[0]->GetState<Correlation>();
    cr->force_cache(0, num, 0, 0, 0);
    auto [a, b, c, r, dy_key] = cr->DyBeaverTripleSet(num);
    return std::make_tuple(a, b, c, r, dy_key);
  });
  auto rank1 = std::async([&] {
    auto cr = context[1]->GetState<Correlation>();
    cr->force_cache(0, 0, num, 0, 0);
    auto [a, b, c, r, dy_key] = cr->DyBeaverTripleGet(num);
    return std::make_tuple(a, b, c, r, dy_key);
  });

  auto [a0, b0, c0, r0, k0] = rank0.get();
  auto [a1, b1, c1, r1, k1] = rank1.get();

  auto a0_val = internal::ExtractVal(a0);
  auto a1_val = internal::ExtractVal(a1);
  auto b0_val = internal::ExtractVal(b0);
  auto b1_val = internal::ExtractVal(b1);
  auto c0_val = internal::ExtractVal(c0);
  auto c1_val = internal::ExtractVal(c1);
  auto r0_val = internal::ExtractVal(r0);
  auto r1_val = internal::ExtractVal(r1);

  auto a = internal::op::Add(absl::MakeSpan(a0_val), absl::MakeSpan(a1_val));
  auto b = internal::op::Add(absl::MakeSpan(b0_val), absl::MakeSpan(b1_val));
  auto c = internal::op::Add(absl::MakeSpan(c0_val), absl::MakeSpan(c1_val));
  auto r = internal::op::Add(absl::MakeSpan(r0_val), absl::MakeSpan(r1_val));

  auto k = k0.val + k1.val;

  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(b[i], b0_val[i]);
    EXPECT_EQ(a[i] * b[i], c[i]);
    EXPECT_EQ(a[i] * k, r[i]);
  }
}

}  // namespace mcpsi
