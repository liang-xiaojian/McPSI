#include "mcpsi/cr/utils/ot_helper.h"

#include <future>

#include "gtest/gtest.h"
#include "mcpsi/context/register.h"
#include "mcpsi/cr/cr.h"
#include "mcpsi/utils/test_util.h"
#include "mcpsi/utils/vec_op.h"

namespace mcpsi::ot {

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

TEST(OtHelperTest, BeaverWork) {
  auto context = TestParam::GetContext();
  const size_t num = 10000;

  auto rank0 = std::async([&] {
    auto cr = context[0]->GetState<Correlation>();
    auto conn = context[0]->GetConnection();
    auto ot_sender = cr->ot_sender_;
    auto ot_receiver = cr->ot_receiver_;

    auto helper = OtHelper(ot_sender, ot_receiver);

    std::vector<internal::PTy> a(num);
    std::vector<internal::PTy> b(num);
    std::vector<internal::PTy> c(num);
    helper.BeaverTriple(conn, absl::MakeSpan(a), absl::MakeSpan(b),
                        absl::MakeSpan(c));
    return std::make_tuple(a, b, c);
  });
  auto rank1 = std::async([&] {
    auto cr = context[1]->GetState<Correlation>();
    auto conn = context[1]->GetConnection();
    auto ot_sender = cr->ot_sender_;
    auto ot_receiver = cr->ot_receiver_;

    auto helper = OtHelper(ot_sender, ot_receiver);

    std::vector<internal::PTy> a(num);
    std::vector<internal::PTy> b(num);
    std::vector<internal::PTy> c(num);
    helper.BeaverTriple(conn, absl::MakeSpan(a), absl::MakeSpan(b),
                        absl::MakeSpan(c));
    return std::make_tuple(a, b, c);
  });

  auto [a0, b0, c0] = rank0.get();
  auto [a1, b1, c1] = rank1.get();

  auto a = internal::op::Add(absl::MakeSpan(a0), absl::MakeSpan(a1));
  auto b = internal::op::Add(absl::MakeSpan(b0), absl::MakeSpan(b1));
  auto c = internal::op::Add(absl::MakeSpan(c0), absl::MakeSpan(c1));

  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(a[i] * b[i], c[i]);
  }
}

TEST(OtHelperTest, BaseVoleWork) {
  auto context = TestParam::GetContext();
  const size_t num = 10000;

  auto rank0 = std::async([&] {
    auto cr = context[0]->GetState<Correlation>();
    auto conn = context[0]->GetConnection();
    auto ot_sender = cr->ot_sender_;
    auto ot_receiver = cr->ot_receiver_;

    auto helper = OtHelper(ot_sender, ot_receiver);

    auto delta = internal::op::Rand(1)[0];
    std::vector<internal::PTy> c(num);
    helper.BaseVoleSend(conn, delta, absl::MakeSpan(c));
    return std::make_tuple(delta, c);
  });
  auto rank1 = std::async([&] {
    auto cr = context[1]->GetState<Correlation>();
    auto conn = context[1]->GetConnection();
    auto ot_sender = cr->ot_sender_;
    auto ot_receiver = cr->ot_receiver_;

    auto helper = OtHelper(ot_sender, ot_receiver);

    std::vector<internal::PTy> a(num);
    std::vector<internal::PTy> b(num);
    helper.BaseVoleRecv(conn, absl::MakeSpan(a), absl::MakeSpan(b));
    return std::make_tuple(a, b);
  });

  auto [delta, c] = rank0.get();
  auto [a, b] = rank1.get();

  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(a[i] * delta + b[i], c[i]);
  }
}

TEST(OtHelperTest, ShuffleWork) {
  auto context = TestParam::GetContext();
  const size_t num = 10000;

  auto rank0 = std::async([&] {
    auto cr = context[0]->GetState<Correlation>();
    auto conn = context[0]->GetConnection();
    auto ot_sender = cr->ot_sender_;
    auto ot_receiver = cr->ot_receiver_;

    auto helper = OtHelper(ot_sender, ot_receiver);

    auto perm = GenPerm(num);
    std::vector<internal::PTy> delta(num);
    helper.ShuffleSend(conn, absl::MakeSpan(perm), absl::MakeSpan(delta));
    return std::make_tuple(perm, delta);
  });
  auto rank1 = std::async([&] {
    auto cr = context[1]->GetState<Correlation>();
    auto conn = context[1]->GetConnection();
    auto ot_sender = cr->ot_sender_;
    auto ot_receiver = cr->ot_receiver_;

    auto helper = OtHelper(ot_sender, ot_receiver);

    std::vector<internal::PTy> a(num);
    std::vector<internal::PTy> b(num);
    helper.ShuffleRecv(conn, absl::MakeSpan(a), absl::MakeSpan(b));
    return std::make_tuple(a, b);
  });

  auto [perm, delta] = rank0.get();
  auto [a, b] = rank1.get();

  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(delta[i] + a[perm[i]] + b[i], internal::PTy(0));
  }
}

}  // namespace mcpsi::ot
