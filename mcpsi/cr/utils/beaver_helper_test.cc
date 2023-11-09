#include "mcpsi/cr/utils/beaver_helper.h"

#include <future>

#include "gtest/gtest.h"
#include "mcpsi/utils/test_util.h"

namespace mcpsi::ot {

TEST(BeaverHelperTest, BeaverWork) {
  auto context = MockContext(2);
  MockInitContext(context);
  const size_t num = 10000;

  auto value = Rand(num);

  auto rank0 = std::async([&] {
    auto cr = context[0]->GetState<Correlation>();
    auto conn = context[0]->GetConnection();
    auto ot_sender = cr->ot_sender_;
    auto ot_receiver = cr->ot_receiver_;

    auto helper = BeaverHelper(ot_sender, ot_receiver);

    std::vector<internal::ATy> a(num, {0, 0});
    std::vector<internal::ATy> b(num, {0, 0});
    std::vector<internal::ATy> c(num, {0, 0});
    helper->BeaverTriple(conn, absl::MakeSpan(a), absl::MakeSpan(b),
                         absl::MakeSpan(c));
    return std::make_tuple(a, b, c);
  });
  auto rank1 = std::async([&] {
    auto cr = context[1]->GetState<Correlation>();
    auto conn = context[1]->GetConnection();
    auto ot_sender = cr->ot_sender_;
    auto ot_receiver = cr->ot_receiver_;

    auto helper = BeaverHelper(ot_sender, ot_receiver);

    std::vector<internal::ATy> a(num, {0, 0});
    std::vector<internal::ATy> b(num, {0, 0});
    std::vector<internal::ATy> c(num, {0, 0});
    helper->BeaverTriple(conn, absl::MakeSpan(a), absl::MakeSpan(b),
                         absl::MakeSpan(c));
    return std::make_tuple(a, b, c);
  });

  auto [a0, b0, c0] = rank0.get();
  auto [a1, b1, c1] = rank1.get();

  auto a = Add(absl::MakeSpan(a0), absl::MakeSpan(a1));
  auto b = Add(absl::MakeSpan(b0), absl::MakeSpan(b1));
  auto c = Add(absl::MakeSpan(c0), absl::MakeSpan(c1));

  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(a[i] * b[i], c[i]);
  }
}

}  // namespace mcpsi::ot
