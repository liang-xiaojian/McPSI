#include <future>

#include "llvm/Support/CommandLine.h"
#include "mcpsi/context/register.h"
#include "mcpsi/ss/protocol.h"
#include "mcpsi/utils/test_util.h"
#include "mcpsi/utils/vec_op.h"

using namespace mcpsi;

// TODO: support separated terminal execution
llvm::cl::opt<std::string> cl_parties(
    "parties", llvm::cl::init("127.0.0.1:39530,127.0.0.1:39531"),
    llvm::cl::desc("server list, format: host1:port1[,host2:port2, ...]"));
llvm::cl::opt<uint32_t> cl_rank("rank", llvm::cl::init(0),
                                llvm::cl::desc("self rank"));

llvm::cl::opt<uint32_t> cl_mode(
    "mode", llvm::cl::init(0),
    llvm::cl::desc("0 for memory mode, 1 for localhost mode"));
llvm::cl::opt<uint32_t> cl_size0("set0", llvm::cl::init(10000),
                                 llvm::cl::desc("the size of set0"));
llvm::cl::opt<uint32_t> cl_size1("set1", llvm::cl::init(10000),
                                 llvm::cl::desc("the size of set1"));
llvm::cl::opt<uint32_t> cl_interset_size(
    "interset", llvm::cl::init(1000),
    llvm::cl::desc("the size of intersection"));

auto mc_psi(size_t n0, size_t n1, size_t interset_size)
    -> std::pair<std::vector<uint64_t>, std::vector<uint64_t>> {
  auto flag = cl_mode.getValue() == 0;  // 0 for memory, otherwise localhost
  auto context = MockContext(2, flag);
  MockInitContext(context);

  std::vector<PTy> force = Rand(interset_size);

  auto rank0 = std::async([&] {
    std::vector<PTy> set0 = Rand(n0);
    memcpy(set0.data(), force.data(), interset_size * sizeof(PTy));

    auto prot = context[0]->GetState<Protocol>();
    SPDLOG_INFO("[P0] upload data");
    auto share0 = prot->SetA(set0);
    auto share1 = prot->GetA(n1);
    auto secret = prot->GetA(n1);
    SPDLOG_INFO("[P0] Begin Circuit-PSI, set0 {} && set1 {}", share0.size(),
                share1.size());
    auto result_s = prot->CPSI(share0, share1, secret);
    SPDLOG_INFO("[P0] interset size {}", result_s.size());
    auto sum_s = prot->SumA(result_s);
    auto result_p = prot->A2P(sum_s);
    auto ret = std::vector<uint64_t>(1);
    ret[0] = result_p[0].GetVal();
    SPDLOG_INFO("[P0] sum is {}", ret[0]);
    return ret;
  });
  auto rank1 = std::async([&] {
    std::vector<PTy> set1 = Rand(n1);
    std::vector<PTy> val1 = Ones(n1);
    memcpy(set1.data(), force.data(), interset_size * sizeof(PTy));

    auto prot = context[1]->GetState<Protocol>();
    SPDLOG_INFO("[P1] upload data");
    auto share0 = prot->GetA(n0);
    auto share1 = prot->SetA(set1);
    auto secret = prot->SetA(val1);
    SPDLOG_INFO("[P1] Begin Circuit-PSI, set0 {} && set1 {}", share0.size(),
                share1.size());
    auto result_s = prot->CPSI(share0, share1, secret);
    SPDLOG_INFO("[P1] interset size {}", result_s.size());
    auto sum_s = prot->SumA(result_s);
    auto result_p = prot->A2P(sum_s);
    auto ret = std::vector<uint64_t>(1);
    ret[0] = result_p[0].GetVal();
    SPDLOG_INFO("[P1] sum is {}", ret[0]);
    return ret;
  });

  auto result0 = rank0.get();
  auto result1 = rank1.get();

  return {result0, result1};
}

int main(int argc, char** argv) {
  llvm::cl::ParseCommandLineOptions(argc, argv);

  size_t size0 = cl_size0.getValue();
  size_t size1 = cl_size1.getValue();
  size_t interset_size = cl_interset_size.getValue();

  if (interset_size > std::min(size0, size1)) {
    std::cout << std::endl
              << "[Warning] interset size (" << interset_size
              << ") is greater than the size of size0 (" << size0
              << ") / size1 (" << size1 << ")" << std::endl;
    interset_size = std::min(size0, size1);
    std::cout << "[Recorrect] set inserset size as " << interset_size
              << std::endl;
  }

  // execute malicious circuit PSI (and sum the result)
  auto [result0, result1] = mc_psi(size0, size1, interset_size);
  std::cout << "Current Task --> P0 with size ( " << size0
            << " ) && P1 with size ( " << size1 << " )" << std::endl;
  std::cout << "P0 result (sum): " << result0[0] << std::endl;
  std::cout << "P1 result (sum): " << result1[0] << std::endl;
  return 0;
}
