#include "mcpsi/cr/utils/vole.h"

#include <future>
#include <set>

#include "gtest/gtest.h"
#include "mcpsi/context/register.h"
#include "mcpsi/cr/cr.h"
#include "mcpsi/utils/test_util.h"
#include "mcpsi/utils/vec_op.h"
#include "vole.h"
#include "yacl/crypto/utils/rand.h"

namespace mcpsi::vole {

namespace yc = yacl::crypto;

struct MpVoleTestParam {
  size_t mp_vole_size;
  size_t noise_num;
};  // namespace yacl::crypto

class MpVoleTest : public ::testing::TestWithParam<MpVoleTestParam> {};

TEST_P(MpVoleTest, Work) {
  auto context = MockContext(2);
  MockInitContext(context);
  const size_t mp_vole_size = GetParam().mp_vole_size;
  const size_t noise_num = GetParam().noise_num;

  auto param = MpParam(mp_vole_size, noise_num);
  param.GenIndexes();
  auto cot = yc::MockCots(param.require_ot_num_, yc::RandU128());

  auto v = Rand(noise_num);
  auto w = Rand(noise_num);

  auto rank0 = std::async([&] {
    auto cr = context[0]->GetState<Correlation>();
    auto conn = context[0]->GetConnection();

    std::vector<internal::PTy> output(mp_vole_size);
    MpVoleSend(conn, cot.send, param, absl::MakeSpan(w),
               absl::MakeSpan(output));
    return output;
  });
  auto rank1 = std::async([&] {
    auto cr = context[1]->GetState<Correlation>();
    auto conn = context[1]->GetConnection();

    std::vector<internal::PTy> output(mp_vole_size);
    MpVoleRecv(conn, cot.recv, param, absl::MakeSpan(v),
               absl::MakeSpan(output));
    return output;
  });

  auto s_output = rank0.get();
  auto r_output = rank1.get();

  std::set<size_t> indexes;
  for (size_t i = 0; i < noise_num; ++i) {
    indexes.insert(i * param.sp_vole_size_ + param.indexes_[i]);
  }

  size_t i = 0;
  size_t j = 0;
  for (; i < mp_vole_size && j < noise_num; ++i) {
    if (s_output[i] != r_output[i]) {
      EXPECT_EQ(s_output[i] - r_output[i], w[j] - v[j]);
      EXPECT_TRUE(indexes.count(i));
      ++j;
    }
  }
  for (; i < mp_vole_size; ++i) {
    EXPECT_EQ(s_output[i], r_output[i]);
  }
}

TEST(WolverineVoleTest, PreWork) {
  auto context = MockContext(2);
  MockInitContext(context);

  auto param = LpnParam::GetPreDefault();
  param.mp_param_.GenIndexes();
  auto cot = yc::MockCots(param.mp_param_.require_ot_num_, yc::RandU128());
  size_t pre_num = param.k_;
  size_t vole_num = param.n_;

  auto pre_a = Rand(pre_num);
  auto pre_b = Rand(pre_num);
  auto deltas = Rand(1);
  auto delta = deltas[0];

  auto pre_c = ScalarMul(delta, absl::MakeSpan(pre_a));
  Add(absl::MakeConstSpan(pre_c), absl::MakeConstSpan(pre_b),
      absl::MakeSpan(pre_c));

  auto rank0 = std::async([&] {
    auto cr = context[0]->GetState<Correlation>();
    auto conn = context[0]->GetConnection();
    std::vector<internal::PTy> c(vole_num, 0);
    WolverineVoleSend(conn, cot.send, param, absl::MakeSpan(pre_c),
                      absl::MakeSpan(c));
    return c;
  });
  auto rank1 = std::async([&] {
    auto cr = context[1]->GetState<Correlation>();
    auto conn = context[1]->GetConnection();
    std::vector<internal::PTy> a(vole_num, 0);
    std::vector<internal::PTy> b(vole_num, 0);
    WolverineVoleRecv(conn, cot.recv, param, absl::MakeSpan(pre_a),
                      absl::MakeSpan(pre_b), absl::MakeSpan(a),
                      absl::MakeSpan(b));
    return std::make_pair(a, b);
  });

  auto c = rank0.get();
  auto [a, b] = rank1.get();

  for (size_t i = 0; i < vole_num; ++i) {
    EXPECT_EQ(a[i] * delta + b[i], c[i]);
  }
}

TEST(WolverineVoleTest, BootStrapWork) {
  auto context = MockContext(2);
  MockInitContext(context);

  auto param = LpnParam::GetDefault();
  param.mp_param_.GenIndexes();
  auto cot = yc::MockCots(param.mp_param_.require_ot_num_, yc::RandU128());
  size_t pre_num = param.k_;
  size_t vole_num = param.n_;

  auto pre_a = Rand(pre_num);
  auto pre_b = Rand(pre_num);
  auto deltas = Rand(1);
  auto delta = deltas[0];

  auto pre_c = ScalarMul(delta, absl::MakeSpan(pre_a));
  Add(absl::MakeConstSpan(pre_c), absl::MakeConstSpan(pre_b),
      absl::MakeSpan(pre_c));

  auto rank0 = std::async([&] {
    auto cr = context[0]->GetState<Correlation>();
    auto conn = context[0]->GetConnection();
    std::vector<internal::PTy> c(vole_num, 0);
    WolverineVoleSend(conn, cot.send, param, absl::MakeSpan(pre_c),
                      absl::MakeSpan(c));
    return c;
  });
  auto rank1 = std::async([&] {
    auto cr = context[1]->GetState<Correlation>();
    auto conn = context[1]->GetConnection();
    std::vector<internal::PTy> a(vole_num, 0);
    std::vector<internal::PTy> b(vole_num, 0);
    WolverineVoleRecv(conn, cot.recv, param, absl::MakeSpan(pre_a),
                      absl::MakeSpan(pre_b), absl::MakeSpan(a),
                      absl::MakeSpan(b));
    return std::make_pair(a, b);
  });

  auto c = rank0.get();
  auto [a, b] = rank1.get();

  for (size_t i = 0; i < vole_num; ++i) {
    EXPECT_EQ(a[i] * delta + b[i], c[i]);
  }
}

INSTANTIATE_TEST_SUITE_P(Works, MpVoleTest,
                         testing::Values(MpVoleTestParam{4, 2},
                                         MpVoleTestParam{5, 2},
                                         MpVoleTestParam{7, 2},
                                         MpVoleTestParam{1 << 8, 63},
                                         MpVoleTestParam{1 << 10, 257}));
}  // namespace mcpsi::vole
