#include "mcpsi/cr/utils/ot_adapter.h"

#include "gtest/gtest.h"
#include "mcpsi/utils/test_util.h"
#include "yacl/base/dynamic_bitset.h"

namespace mcpsi::ot {

TEST(OtAdapterTest, ROT) {
  size_t num = 10000;
  std::vector<uint128_t> recv_data(num);
  std::vector<std::array<uint128_t, 2>> send_data(num);
  yacl::dynamic_bitset<uint128_t> choices(num);
  auto lctxs = SetupWorld(2);
  auto rank0 = std::async([&] {
    auto otSender = std::make_shared<YaclKosOtAdapter>(lctxs[0], true);
    otSender->OneTimeSetup();
    otSender->send_rrot(absl::MakeSpan(send_data));
  });
  auto rank1 = std::async([&] {
    auto otReceiver = std::make_shared<YaclKosOtAdapter>(lctxs[1], false);
    otReceiver->OneTimeSetup();
    otReceiver->recv_rrot(absl::MakeSpan(recv_data), choices);
  });
  rank0.get();
  rank1.get();

  for (size_t i = 0; i < num; ++i) {
    EXPECT_EQ(recv_data[i], send_data[i][choices[i]]);
    EXPECT_NE(recv_data[i], send_data[i][1 - choices[i]]);
  }
};

TEST(OtAdapterTest, COT) {
  size_t num = 10000;
  std::vector<uint128_t> recv_data(num);
  std::vector<uint128_t> send_data(num);
  yacl::dynamic_bitset<uint128_t> choices(num);

  auto lctxs = SetupWorld(2);
  auto rank0 = std::async([&] {
    auto otSender = std::make_shared<YaclKosOtAdapter>(lctxs[0], true);
    otSender->OneTimeSetup();
    otSender->send_rcot(absl::MakeSpan(send_data));
    return otSender->GetDelta();
  });
  auto rank1 = std::async([&] {
    auto otReceiver = std::make_shared<YaclKosOtAdapter>(lctxs[1], false);
    otReceiver->OneTimeSetup();
    otReceiver->recv_rcot(absl::MakeSpan(recv_data), choices);
  });
  auto delta = rank0.get();
  rank1.get();

  for (size_t i = 0; i < num; ++i) {
    if (choices[i] == 0) {
      EXPECT_EQ(send_data[i] ^ recv_data[i], uint128_t(0));
    } else {
      EXPECT_EQ(send_data[i] ^ recv_data[i], delta);
    }
  }
};

}  // namespace mcpsi::ot