#include <future>

#include "mcpsi/context/register.h"
#include "mcpsi/ss/protocol.h"
#include "mcpsi/utils/test_util.h"

using namespace test;

std::vector<uint64_t> intersection(std::vector<uint64_t>& lhs,
                                   std::vector<uint64_t>& rhs) {
  std::sort(lhs.begin(), lhs.end());
  std::sort(rhs.begin(), rhs.end());

  std::vector<uint64_t> ret;
  std::set_intersection(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                        std::back_inserter(ret));

  return ret;
}

auto toy_psi() -> std::pair<std::vector<uint64_t>, std::vector<uint64_t>> {
  auto context = MockContext(2);
  MockInitContext(context);
  auto rank0 = std::async([&] {
    auto prot = context[0]->GetState<Protocol>();
    std::vector<PTy> set0{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto share0 = prot->SetA(set0);
    auto share1 = prot->GetA(10);

    auto shuffle0 = prot->ShuffleA(share0);
    auto shuffle1 = prot->ShuffleA(share1);

    auto reveal0 = prot->A2G(shuffle0);
    auto reveal1 = prot->A2G(shuffle1);

    std::vector<uint64_t> lhs(10);
    std::vector<uint64_t> rhs(10);
    for (size_t i = 0; i < 10; ++i) {
      lhs[i] = reveal0[i].Get<uint64_t>();
      rhs[i] = reveal1[i].Get<uint64_t>();
    }
    return intersection(lhs, rhs);
  });
  auto rank1 = std::async([&] {
    auto prot = context[1]->GetState<Protocol>();
    std::vector<PTy> set1{2, 4, 6, 8, 10, 12, 14, 16, 18, 20};
    auto share0 = prot->GetA(10);
    auto share1 = prot->SetA(set1);

    auto shuffle0 = prot->ShuffleA(share0);
    auto shuffle1 = prot->ShuffleA(share1);

    auto reveal0 = prot->A2G(shuffle0);
    auto reveal1 = prot->A2G(shuffle1);

    std::vector<uint64_t> lhs(10);
    std::vector<uint64_t> rhs(10);
    for (size_t i = 0; i < 10; ++i) {
      lhs[i] = reveal0[i].Get<uint64_t>();
      rhs[i] = reveal1[i].Get<uint64_t>();
    }
    return intersection(lhs, rhs);
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
  std::cout << "Intersection  --> {2, 4, 6, 8, 10} (5 elements)";
  std::cout << std::endl;

  auto [result0, result1] = toy_psi();

  std::cout << std::endl;
  std::cout << "P0 result: ( size :" << result0.size() << " )" << std::endl;
  for (const auto e : result0) {
    std::cout << e << " ";
  }
  std::cout << std::endl;

  std::cout << "P1 result: ( size :" << result1.size() << " )" << std::endl;
  for (const auto e : result1) {
    std::cout << e << " ";
  }
  std::cout << std::endl;
  return 0;
}