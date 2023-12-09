#include "mcpsi/cr/utils/vole_adapter.h"

#include <future>
#include <set>

#include "gtest/gtest.h"
#include "mcpsi/context/register.h"
#include "mcpsi/cr/cr.h"
#include "mcpsi/cr/utils/ot_adapter.h"
#include "mcpsi/cr/utils/ot_helper.h"
#include "mcpsi/cr/utils/vole.h"
#include "mcpsi/utils/test_util.h"
#include "mcpsi/utils/vec_op.h"
#include "vole.h"
#include "yacl/crypto/utils/rand.h"

namespace mcpsi::vole {

namespace yc = yacl::crypto;

struct VoleTestParam {
  size_t num;
};

class VoleAdapterTest : public ::testing::TestWithParam<VoleTestParam> {};

// TODO: Fix Link Problem !!!
TEST_P(VoleAdapterTest, Work) {
  auto context = MockContext(2);
  MockInitContext(context);
  const size_t vole_num = GetParam().num;
  auto deltas = vec64::Rand(1);
  auto delta = deltas[0];

  auto rank0 = std::async([&] {
    auto cr = context[0]->GetState<Correlation>();
    auto conn = context[0]->GetConnection();

    auto voleSender =
        std::make_shared<WolverineVoleAdapter>(conn, cr->ot_sender_, delta);

    std::vector<internal::PTy> c(vole_num);
    voleSender->rsend(absl::MakeSpan(c));
    return c;
  });
  auto rank1 = std::async([&] {
    auto cr = context[1]->GetState<Correlation>();
    auto conn = context[1]->GetConnection();

    auto voleReceiver =
        std::make_shared<WolverineVoleAdapter>(conn, cr->ot_receiver_);
    std::vector<internal::PTy> a(vole_num);
    std::vector<internal::PTy> b(vole_num);
    voleReceiver->rrecv(absl::MakeSpan(a), absl::MakeSpan(b));
    return std::make_pair(a, b);
  });

  auto c = rank0.get();
  auto [a, b] = rank1.get();

  for (size_t i = 0; i < vole_num; ++i) {
    EXPECT_EQ(a[i] * delta + b[i], c[i]);
  }
}

INSTANTIATE_TEST_SUITE_P(Works, VoleAdapterTest,
                         testing::Values(VoleTestParam{2}, VoleTestParam{10},
                                         VoleTestParam{1000},
                                         VoleTestParam{1 << 20}));
}  // namespace mcpsi::vole
