#include <future>

#include "mcpsi/context/register.h"
#include "mcpsi/ss/protocol.h"
#include "mcpsi/utils/test_util.h"
#include "mcpsi/utils/vec_op.h"

using namespace mcpsi;

auto toy_mc_psi() -> std::pair<std::vector<uint64_t>, std::vector<uint64_t>> {
  auto context = MockContext(2);
  MockSetupContext(context);
  auto rank0 = std::async([&] {
    auto prot = context[0]->GetState<Protocol>();
    std::vector<PTy> set0{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto share0 = prot->SetA(set0);
    auto share1 = prot->GetA(10);
    auto secret = prot->GetA(10);

    auto result_s = prot->CPSI(share0, share1, secret);
    auto result_p = prot->A2P(result_s);
    auto ret = std::vector<uint64_t>(1);
    ret[0] = result_p[0].GetVal();
    return ret;
  });
  auto rank1 = std::async([&] {
    auto prot = context[1]->GetState<Protocol>();
    std::vector<PTy> set1{2, 4, 6, 8, 10, 12, 14, 16, 18, 2};
    std::vector<PTy> val1{3, 6, 9, 12, 15, 18, 21, 24, 27, 30};

    auto share0 = prot->GetA(10);
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
  std::cout << "Experiment: " << std::endl;
  std::cout << "Set0          --> {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}";
  std::cout << std::endl;
  std::cout << "Set1          --> {2, 4, 6, 8, 10, 12, 14, 16, 18,  2}";
  std::cout << std::endl;
  std::cout << "                   |  |  |  |   |   |   |   |   |   | ";
  std::cout << std::endl;
  std::cout << "                   v  v  v  v   v   v   v   v   v   v ";
  std::cout << std::endl;
  std::cout << "                 { 3, 6, 9, 12, 15, 18, 21, 24, 27, 30}";
  std::cout << std::endl << std::endl;
  std::cout << "sum( result ) --> 3 + 6 + 9 + 12 + 15 + 30 = 75";
  std::cout << std::endl;

  // execute malicious circuit PSI (and sum the result)
  auto [result0, result1] = toy_mc_psi();

  std::cout << std::endl;
  std::cout << "P0 result (sum): " << result0[0] << std::endl;
  std::cout << "P1 result (sum): " << result1[0] << std::endl;

  return 0;
}
