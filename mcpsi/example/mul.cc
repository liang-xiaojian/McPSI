#include <chrono>
#include <future>
#include <random>

#include "llvm/Support/CommandLine.h"
#include "mcpsi/context/register.h"
#include "mcpsi/ss/protocol.h"
#include "mcpsi/utils/test_util.h"
#include "mcpsi/utils/vec_op.h"
#include "yacl/link/link.h"
#include "yacl/utils/parallel.h"

using namespace mcpsi;

// -------- MACRO ---------

#define TIMER_START(name) \
  auto name##_begin = std::chrono::high_resolution_clock::now();

#define TIMER_END(name) \
  auto name##_end = std::chrono::high_resolution_clock::now();

#define TIMER_PRINT(name)                                                  \
  auto name##_elapse = name##_end - name##_begin;                          \
  double name##_ms =                                                       \
      std::chrono::duration_cast<std::chrono::milliseconds>(name##_elapse) \
          .count();                                                        \
  SPDLOG_INFO("[P{}](TIMER) {} need {} ms (or {} s)", rank,                \
              std::string(#name), name##_ms, name##_ms / 1000);

#define COMM_START(name)                                      \
  auto name##_st_start = lctx->GetStats();                    \
  int64_t name##_send_bytes = name##_st_start->sent_bytes;    \
  int64_t name##_send_action = name##_st_start->sent_actions; \
  int64_t name##_recv_bytes = name##_st_start->recv_bytes;    \
  int64_t name##_recv_action = name##_st_start->recv_actions;

#define COMM_END(name)                                                   \
  auto name##_st_end = lctx->GetStats();                                 \
  name##_send_bytes = name##_st_end->sent_bytes - name##_send_bytes;     \
  name##_send_action = name##_st_end->sent_actions - name##_send_action; \
  name##_recv_bytes = name##_st_end->recv_bytes - name##_recv_bytes;     \
  name##_recv_action = name##_st_end->recv_actions - name##_recv_action;

#define COMM_PRINT(name)                                                     \
  SPDLOG_INFO(                                                               \
      "[P{}](COMM) {} send bytes: {} && send actions: {} && recv bytes: {} " \
      "&& "                                                                  \
      "recv actions: {}",                                                    \
      rank, std::string(#name), name##_send_bytes, name##_send_action,       \
      name##_recv_bytes, name##_recv_action);

// ---------- CL -----------

llvm::cl::opt<std::string> cl_parties(
    "parties", llvm::cl::init("127.0.0.1:39530,127.0.0.1:39531"),
    llvm::cl::desc("server list, format: host1:port1[,host2:port2, ...]"));
llvm::cl::opt<uint32_t> cl_rank("rank", llvm::cl::init(0),
                                llvm::cl::desc("self rank"));
llvm::cl::opt<uint32_t> cl_CR(
    "CR", llvm::cl::init(0),
    llvm::cl::desc("0 for prg-based correlated randomness, 1 for real "
                   "correlated randomness"));
llvm::cl::opt<uint32_t> cl_cache(
    "cache", llvm::cl::init(0),
    llvm::cl::desc(
        "0 for no cache, 1 for cache (pre-compute offline randomness)"));
llvm::cl::opt<uint32_t> cl_mode(
    "mode", llvm::cl::init(0),
    llvm::cl::desc("0 for memory mode, 1 for socket mode"));
llvm::cl::opt<uint32_t> cl_interset_size(
    "interset", llvm::cl::init(1000),
    llvm::cl::desc("the size of intersection"));

llvm::cl::opt<uint32_t> cl_thread("thread", llvm::cl::init(1),
                                  llvm::cl::desc("the number of threads"));

auto m_mul(const std::shared_ptr<yacl::link::Context> &lctx,
           absl::Span<PTy> data, bool CR_mode = false, bool cache = true) {
  auto rank = lctx->Rank();

  SPDLOG_INFO("[P{}] works with {} threads", rank, yacl::get_num_threads());
  SPDLOG_INFO("[P{}] having set0 (with size {}), working mode: {} && {}", rank,
              data.size(),
              (CR_mode ? std::string("Real Correlated Randomness")
                       : std::string("Fake Correlated Randomness")),
              (cache ? std::string("Cache") : std::string("No Cache")));
  SPDLOG_INFO("[P{}] Initializing Context", rank);

  // ---- MARK ----
  COMM_START(setup);
  TIMER_START(setup);
  auto context = std::make_shared<Context>(lctx);
  SetupContext(context, CR_mode);
  auto prot = context->GetState<Protocol>();
  TIMER_END(setup);
  TIMER_PRINT(setup);
  COMM_END(setup);
  COMM_PRINT(setup);

  // TIMER_START(timer_name);
  // // the code ... such as, prot->Add(x,y);
  // TIMER_END(timer_name);
  // TIMER_PRINT(timer_name);

  // For your information, "cache mode" would try to pre-compute the correlated
  // randomness for Circuit-PSI
  // --- BEGIN CACHE ---
  if (cache) {
    std::vector<PTy> empty_data(data.size());
    auto share0 = (rank == 0 ? prot->SetA(empty_data, true)
                             : prot->GetA(empty_data.size(), true));
    auto share1 = (rank == 1 ? prot->SetA(empty_data, true)
                             : prot->GetA(empty_data.size(), true));
    auto mul = prot->Mul(share0, share1, true);
    auto sum = prot->SumA(mul, true);
    [[maybe_unused]] auto result_p = prot->A2P(sum, true);
    // ---- MARK ----
    COMM_START(offline);
    TIMER_START(offline);
    context->GetState<Correlation>()->force_cache();
    SPDLOG_INFO("[P{}] cache all finished!", rank);
    TIMER_END(offline);
    TIMER_PRINT(offline);
    COMM_END(offline);
    COMM_PRINT(offline);
  }
  // --- END CACHE ---

  // --- MARK
  COMM_START(online);
  TIMER_START(online);
  SPDLOG_INFO("[P{}] uploading data", rank);
  auto share0 = (rank == 0 ? prot->SetA(data) : prot->GetA(data.size()));
  auto share1 = (rank == 1 ? prot->SetA(data) : prot->GetA(data.size()));
  SPDLOG_INFO("[P{}] Then, executing mul with size {}", rank, data.size());

  COMM_START(mul);
  TIMER_START(mul);
  auto mul = prot->Mul(share0, share1);
  TIMER_END(mul);
  TIMER_PRINT(mul);
  COMM_END(mul);
  COMM_PRINT(mul);

  COMM_START(sumA);
  TIMER_START(sumA);
  auto sum = prot->SumA(mul);
  TIMER_END(sumA);
  TIMER_PRINT(sumA);
  COMM_END(sumA);
  COMM_PRINT(sumA);

  auto result_p = prot->A2P(sum);

  TIMER_END(online);
  TIMER_PRINT(online);
  COMM_END(online);
  COMM_PRINT(online);

  typedef decltype(std::declval<internal::PTy>().GetVal()) INTEGER;
  auto ret = std::vector<INTEGER>(1);
  ret[0] = result_p[0].GetVal();

  SPDLOG_INFO("[P{}] square sum is {}", rank, ret[0]);
  return ret;
}

struct ArgPack {
  uint32_t interset_size;
  uint32_t CR_mode;
  uint32_t cache;
  uint128_t seed;

  bool operator==(const ArgPack &other) const {
    return (interset_size == other.interset_size) &&
           (CR_mode == other.CR_mode) && (cache == other.cache);
  }

  bool operator!=(const ArgPack &other) const { return !(*this == other); }
};

bool SyncTask(const std::shared_ptr<yacl::link::Context> &lctx,
              uint32_t interset_size, uint32_t CR_mode, uint32_t cache,
              uint128_t &seed) {
  uint128_t tmp_seed = yacl::crypto::SecureRandU128();
  ArgPack tmp = {interset_size, CR_mode, cache, tmp_seed};
  auto bv = yacl::ByteContainerView(&tmp, sizeof(tmp));

  ArgPack remote;
  if (lctx->Rank()) {
    lctx->SendAsync(lctx->NextRank(), bv, "Sync0");
    auto buf = lctx->Recv(lctx->NextRank(), "Sync1");
    YACL_ENFORCE(buf.size() == sizeof(ArgPack));
    memcpy(&remote, buf.data(), buf.size());

  } else {
    auto buf = lctx->Recv(lctx->NextRank(), "Sync0");
    lctx->SendAsync(lctx->NextRank(), bv, "Sync1");
    YACL_ENFORCE(buf.size() == sizeof(ArgPack));
    memcpy(&remote, buf.data(), buf.size());
  }

  YACL_ENFORCE(remote == tmp);

  seed = tmp_seed ^ remote.seed;
  return true;
}

std::shared_ptr<yacl::link::Context> MakeLink(const std::string &parties,
                                              size_t rank) {
  yacl::link::ContextDesc lctx_desc;
  std::vector<std::string> hosts = absl::StrSplit(parties, ',');
  for (size_t rank = 0; rank < hosts.size(); rank++) {
    const auto id = fmt::format("party{}", rank);
    lctx_desc.parties.emplace_back(id, hosts[rank]);
  }
  lctx_desc.throttle_window_size = 0;
  auto lctx = yacl::link::FactoryBrpc().CreateContext(lctx_desc, rank);
  lctx->ConnectToMesh();
  return lctx;
}

int main(int argc, char **argv) {
  llvm::cl::ParseCommandLineOptions(argc, argv);

  bool mem_mode = cl_mode.getValue() == 0;
  bool CR_mode = cl_CR.getValue();
  bool cache = cl_cache.getValue();

  size_t interset_size = cl_interset_size.getValue();
  size_t thread = cl_thread.getValue();

  yacl::set_num_threads(thread);

  if (mem_mode == true) {
    // execute malicious circuit PSI (and sum the result)

    auto data = OP::Rand(interset_size);

    auto lctxs = SetupWorld(2);
    auto task0 = std::async(
        [&] { return m_mul(lctxs[0], absl::MakeSpan(data), CR_mode, cache); });
    auto task1 = std::async(
        [&] { return m_mul(lctxs[1], absl::MakeSpan(data), CR_mode, cache); });
    auto result0 = task0.get();
    auto result1 = task1.get();
    std::cout << "P0 result (sum): " << result0[0] << std::endl;
    std::cout << "P1 result (sum): " << result1[0] << std::endl;
  } else {
    auto lctx = MakeLink(cl_parties.getValue(), cl_rank.getValue());
    uint128_t seed = 0;
    YACL_ENFORCE(SyncTask(lctx, interset_size, CR_mode, cache, seed));
    auto data = OP::Rand(seed, interset_size);

    auto res = m_mul(lctx, absl::MakeSpan(data), CR_mode, cache);
    std::cout << "P" << cl_rank.getValue() << " result (sum): " << res[0]
              << std::endl;
  }

  return 0;
}
