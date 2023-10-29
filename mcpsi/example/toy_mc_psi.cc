#include <future>

#include "mcpsi/context/register.h"
#include "mcpsi/ss/protocol.h"
#include "mcpsi/utils/test_util.h"
#include "mcpsi/utils/vec_op.h"

using namespace mcpsi;

std::vector<uint64_t> intersection(std::vector<uint64_t>& lhs,
                                   std::vector<uint64_t>& rhs) {
  std::sort(lhs.begin(), lhs.end());
  std::sort(rhs.begin(), rhs.end());

  std::vector<uint64_t> ret;
  std::set_intersection(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                        std::back_inserter(ret));

  return ret;
}

auto toy_mc_psi() -> std::pair<std::vector<uint64_t>, std::vector<uint64_t>> {
  auto context = MockContext(2);
  MockInitContext(context);
  auto rank0 = std::async([&] {
    auto prot = context[0]->GetState<Protocol>();
    std::vector<PTy> set0{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto share0 = prot->SetA(set0);
    auto share1 = prot->GetA(10);
    auto secret = prot->GetA(10);

    auto perm0 = GenPerm(10);
    auto perm1 = GenPerm(10);
    auto shuffle0 = prot->ShuffleA(share0, perm0);
    auto shuffle1 = prot->ShuffleA(share1, perm1);
    auto shuffle_secret = prot->ShuffleA(secret, perm1);

    auto reveal0 = prot->A2G(shuffle0);
    auto reveal1 = prot->A2G(shuffle1);

    std::vector<uint64_t> lhs(10);
    std::vector<uint64_t> rhs(10);
    for (size_t i = 0; i < 10; ++i) {
      lhs[i] = reveal0[i].Get<uint64_t>();
      rhs[i] = reveal1[i].Get<uint64_t>();
    }
    auto psi = intersection(lhs, rhs);

    std::vector<size_t> indexes;
    for (auto const e : psi) {
      auto ptr = std::find(reveal1.begin(), reveal1.end(), GTy(e));
      YACL_ENFORCE(ptr != reveal1.end());
      auto idx = ptr - reveal1.begin();
      indexes.emplace_back(idx);
    }

    auto selected = prot->FilterA(absl::MakeConstSpan(shuffle_secret),
                                  absl::MakeConstSpan(indexes));

    auto result_s = prot->SumA(selected);
    auto result_p = prot->A2P(result_s);
    auto ret = std::vector<uint64_t>(1);
    ret[0] = result_p[0].GetVal();
    return ret;
  });
  auto rank1 = std::async([&] {
    auto prot = context[1]->GetState<Protocol>();
    std::vector<PTy> set1{2, 4, 6, 8, 10, 12, 14, 16, 18, 20};
    std::vector<PTy> val1{3, 6, 9, 12, 15, 18, 21, 24, 27, 30};

    auto share0 = prot->GetA(10);
    auto share1 = prot->SetA(set1);
    auto secret = prot->SetA(val1);

    auto perm0 = GenPerm(10);
    auto perm1 = GenPerm(10);
    auto shuffle0 = prot->ShuffleA(share0, perm0);
    auto shuffle1 = prot->ShuffleA(share1, perm1);
    auto shuffle_secret = prot->ShuffleA(secret, perm1);

    auto reveal0 = prot->A2G(shuffle0);
    auto reveal1 = prot->A2G(shuffle1);

    std::vector<uint64_t> lhs(10);
    std::vector<uint64_t> rhs(10);
    for (size_t i = 0; i < 10; ++i) {
      lhs[i] = reveal0[i].Get<uint64_t>();
      rhs[i] = reveal1[i].Get<uint64_t>();
    }
    auto psi = intersection(lhs, rhs);

    std::vector<size_t> indexes;
    for (auto const e : psi) {
      auto ptr = std::find(reveal1.begin(), reveal1.end(), GTy(e));
      YACL_ENFORCE(ptr != reveal1.end());
      auto idx = ptr - reveal1.begin();
      indexes.emplace_back(idx);
    }

    auto selected = prot->FilterA(absl::MakeConstSpan(shuffle_secret),
                                  absl::MakeConstSpan(indexes));

    auto result_s = prot->SumA(selected);
    auto result_p = prot->A2P(result_s);
    auto ret = std::vector<uint64_t>(1);
    ret[0] = result_p[0].GetVal();
    return ret;
  });

  auto result0 = rank0.get();
  auto result1 = rank1.get();

  return {result0, result1};
}

int main() {
  std::cout << "Experiment: " << std::endl;
  std::cout << "Set0          --> {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}";
  std::cout << std::endl;
  std::cout << "Set1          --> {2, 4, 6, 8, 10, 12, 14, 16, 18, 20}";
  std::cout << std::endl;
  std::cout << "                   |  |  |  |   |   |   |   |   |   | ";
  std::cout << std::endl;
  std::cout << "                   v  v  v  v   v   v   v   v   v   v ";
  std::cout << std::endl;
  std::cout << "                 { 3, 6, 9, 12, 15, 18, 21, 24, 27, 30}";
  std::cout << std::endl << std::endl;
  std::cout << "sum( result ) --> 3 + 6 + 9 + 12 + 15 = 45";
  std::cout << std::endl;

  auto [result0, result1] = toy_mc_psi();

  std::cout << std::endl;
  std::cout << "P0 result (sum): " << result0[0] << std::endl;
  std::cout << "P1 result (sum): " << result1[0] << std::endl;

  return 0;
}
