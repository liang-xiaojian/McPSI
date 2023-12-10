#include <future>
#include <unordered_set>

#include "mcpsi/context/register.h"
#include "mcpsi/ss/protocol.h"
#include "mcpsi/utils/test_util.h"

using namespace mcpsi;

// return the set such that all elements "e"
// satisyfing "e in set0" && "e in set1"
std::vector<size_t> intersection(const std::shared_ptr<yc::EcGroup>& Ggroup,
                                 std::vector<GTy>& set0,
                                 std::vector<GTy>& set1) {
  auto group_hash = [&Ggroup](const GTy& val) {
    return Ggroup->HashPoint(val);
  };
  auto group_equal = [&Ggroup](const GTy& lhs, const GTy& rhs) {
    return Ggroup->PointEqual(lhs, rhs);
  };

  std::unordered_set<GTy, decltype(group_hash), decltype(group_equal)> hash_set(
      set0.begin(), set0.end(), 2, group_hash, group_equal);

  // save hash value
  std::vector<size_t> ret;
  for (const auto& e : set1) {
    if (hash_set.count(e)) {
      ret.emplace_back(group_hash(e));
    }
  }
  return ret;
}

auto toy_psi() -> std::pair<std::vector<size_t>, std::vector<size_t>> {
  auto context = MockContext(2);
  MockSetupContext(context);
  auto rank0 = std::async([&] {
    auto prot = context[0]->GetState<Protocol>();
    std::vector<PTy> set0{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto share0 = prot->SetA(set0);
    auto share1 = prot->GetA(10);

    auto shuffle0 = prot->ShuffleA(share0);
    auto shuffle1 = prot->ShuffleA(share1);

    auto reveal0 = prot->A2G(shuffle0);
    auto reveal1 = prot->A2G(shuffle1);

    return intersection(prot->GetGroup(), reveal0, reveal1);
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

    return intersection(prot->GetGroup(), reveal0, reveal1);
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
  for (const auto& e : result0) {
    std::cout << e << " ";
  }
  std::cout << std::endl;

  std::cout << "P1 result: ( size :" << result1.size() << " )" << std::endl;
  for (const auto& e : result1) {
    std::cout << e << " ";
  }
  std::cout << std::endl;
  return 0;
}