#include <future>

#include "mcpsi/context/register.h"
#include "mcpsi/ss/protocol.h"
#include "mcpsi/utils/test_util.h"
#include "mcpsi/utils/vec_op.h"

using namespace mcpsi;

auto mc_psi(size_t n0, size_t n1)
    -> std::pair<std::vector<uint64_t>, std::vector<uint64_t>> {
  auto context = MockContext(2);
  MockInitContext(context);

  size_t min_num = std::min(n0, n1) / 4;
  std::vector<PTy> force = Rand(min_num);

  auto rank0 = std::async([&] {
    std::vector<PTy> set0 = Rand(n0);
    memcpy(set0.data(), force.data(), min_num * sizeof(PTy));

    auto prot = context[0]->GetState<Protocol>();
    auto share0 = prot->SetA(set0);
    auto share1 = prot->GetA(n1);
    auto secret = prot->GetA(n1);

    auto result_s = prot->CPSI(share0, share1, secret);
    auto result_p = prot->A2P(result_s);
    auto ret = std::vector<uint64_t>(1);
    ret[0] = result_p[0].GetVal();
    return ret;
  });
  auto rank1 = std::async([&] {
    std::vector<PTy> set1 = Rand(n1);
    std::vector<PTy> val1 = Rand(n1);
    memcpy(set1.data(), force.data(), min_num * sizeof(PTy));

    auto prot = context[1]->GetState<Protocol>();

    auto share0 = prot->GetA(n0);
    auto share1 = prot->SetA(set1);
    auto secret = prot->SetA(val1);

    auto result_s = prot->CPSI(share0, share1, secret);
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
  size_t n0 = 10000;
  size_t n1 = 10000;
  // execute malicious circuit PSI (and sum the result)
  auto [result0, result1] = mc_psi(n0, n1);

  std::cout << "Current Task --> P0 with size ( " << n0
            << " ) && P1 with size ( " << n1 << " )" << std::endl;
  std::cout << "P0 result (sum): " << result0[0] << std::endl;
  std::cout << "P1 result (sum): " << result1[0] << std::endl;
  return 0;
}
