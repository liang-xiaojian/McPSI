#include "mcpsi/cr/utils/linear_code.h"

#include <vector>

#include "gtest/gtest.h"
#include "mcpsi/ss/type.h"
#include "mcpsi/utils/vec_op.h"

namespace mcpsi::code {

TEST(Llc, LlcWorks) {
  // GIVEN
  uint128_t seed = yacl::crypto::RandSeed();
  uint32_t n = 102400;
  uint32_t k = 1024;
  LocalLinearCode<10> llc(seed, n, k);
  auto input = vec64::Rand(k);
  std::vector<internal::PTy> out(n);
  std::vector<internal::PTy> check(n);
  // WHEN
  llc.Encode(input, absl::MakeSpan(out));
  llc.Encode(input, absl::MakeSpan(check));

  uint32_t zero_counter = 0;
  for (uint32_t i = 0; i < n; ++i) {
    EXPECT_EQ(out[i], check[i]);
    if (out[i] == 0) {
      zero_counter++;
    }
  }

  EXPECT_LE(zero_counter, 2);
}

}  // namespace mcpsi::code