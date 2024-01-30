#include <chrono>
#include <future>

#include "llvm/Support/CommandLine.h"
#include "mcpsi/context/register.h"
#include "mcpsi/ss/protocol.h"
#include "mcpsi/utils/test_util.h"
#include "mcpsi/utils/vec_op.h"
#include "yacl/link/link.h"

using namespace mcpsi;

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

llvm::cl::opt<std::string> cl_parties(
    "parties", llvm::cl::init("127.0.0.1:39530,127.0.0.1:39531"),
    llvm::cl::desc("server list, format: host1:port1[,host2:port2, ...]"));
llvm::cl::opt<uint32_t> cl_rank("rank", llvm::cl::init(0),
                                llvm::cl::desc("self rank"));
llvm::cl::opt<uint32_t> cl_offline(
    "offline", llvm::cl::init(0),
    llvm::cl::desc("0 for real offline, 1 for fake offline"));
llvm::cl::opt<uint32_t> cl_cache("cache", llvm::cl::init(0),
                                 llvm::cl::desc("0 for no cache, 1 for cache"));
llvm::cl::opt<uint32_t> cl_fairness(
    "fairness", llvm::cl::init(0),
    llvm::cl::desc("0 for no fairness, 1 for fairness"));
llvm::cl::opt<uint32_t> cl_mode(
    "mode", llvm::cl::init(0),
    llvm::cl::desc("0 for memory mode, 1 for socket mode"));
llvm::cl::opt<uint32_t> cl_size0("set0", llvm::cl::init(10000),
                                 llvm::cl::desc("the size of set0"));
llvm::cl::opt<uint32_t> cl_size1("set1", llvm::cl::init(10000),
                                 llvm::cl::desc("the size of set1"));
llvm::cl::opt<uint32_t> cl_interset_size(
    "interset", llvm::cl::init(1000),
    llvm::cl::desc("the size of intersection"));

auto mc_psi(const std::shared_ptr<yacl::link::Context>& lctx,
            absl::Span<PTy> set0, absl::Span<PTy> set1, absl::Span<PTy> val1,
            bool offline = true, bool cache = true, bool fairness = false) {
  auto rank = lctx->Rank();

  if (rank == 0) {
    SPDLOG_INFO(
        "[P{}] having set0 (with size {}), working mode: {} && {} && {}", rank,
        set0.size(),
        (offline ? std::string("Real Correlation")
                 : std::string("Fake Correlation")),
        (cache ? std::string("Cache") : std::string("No Cache")),
        (fairness ? std::string("Fairness") : std::string("No Fairness")));
  } else {
    SPDLOG_INFO(
        "[P{}] having set1 && data (with size {}), working mode: {} & {} & {}",
        rank, set1.size(),
        (offline ? std::string("Real Correlation")
                 : std::string("Fake Correlation")),
        (cache ? std::string("Cache") : std::string("No Cache")),
        (fairness ? std::string("Fairness") : std::string("No Fairness")));
  }
  SPDLOG_INFO("[P{}] Initializing Context", rank);
  auto context = std::make_shared<Context>(lctx);
  SetupContext(context, offline);
  auto prot = context->GetState<Protocol>();

  // G-group
  auto Ggroup = prot->GetGroup();
  // Hash functor
  auto group_hash = [&Ggroup](const GTy& val) {
    return Ggroup->HashPoint(val);
  };
  // Equal functor
  auto group_equal = [&Ggroup](const GTy& lhs, const GTy& rhs) {
    return Ggroup->PointEqual(lhs, rhs);
  };

  // TIMER_START(timer_name);
  // // the code ... such as, prot->Add(x,y);
  // TIMER_END(timer_name);
  // TIMER_PRINT(timer_name);

  if (cache) {
    std::vector<PTy> empty_set0(set0.size());
    std::vector<PTy> empty_set1(set1.size());
    std::vector<PTy> empty_val1(val1.size());
    auto share0 = (rank == 0 ? prot->SetA(empty_set0, true)
                             : prot->GetA(empty_set0.size(), true));
    auto share1 = (rank == 1 ? prot->SetA(empty_set1, true)
                             : prot->GetA(empty_set1.size(), true));
    auto secret = (rank == 1 ? prot->SetA(empty_val1, true)
                             : prot->GetA(empty_val1.size(), true));
    // auto result_s = prot->CPSI(share0, share1, secret, true);
    // same as CPSI with cache
    auto shuffle0 = prot->ShuffleA(share0, true);
    auto [shuffle1, shuffle_data] = prot->ShuffleA(share1, secret, true);
    // reveal G-share
    if (fairness) {
      auto [scalar_a, bits] = prot->RandFairA(1, true);
      auto reveal0 = prot->ScalarA2G(scalar_a[0], shuffle0, true);
      auto reveal1 = prot->A2G(shuffle1, true);
      auto scalar_p = prot->FairA2P(scalar_a, bits, true);
    } else {
      auto reveal0 = prot->A2G(shuffle0, true);
      auto reveal1 = prot->A2G(shuffle1, true);
    }

    auto indexes = std::vector<size_t>(secret.size());
    auto result_s = prot->FilterA(absl::MakeConstSpan(shuffle_data),
                                  absl::MakeConstSpan(indexes), true);
    auto sum_s = prot->SumA(result_s, true);
    [[maybe_unused]] auto result_p = prot->A2P(sum_s, true);
    SPDLOG_INFO("[P{}] start cache all correlation", rank);
    context->GetState<Correlation>()->force_cache();
    SPDLOG_INFO("[P{}] cache all finished!", rank);
  }

  SPDLOG_INFO("[P{}] uploading data", rank);
  auto share0 = (rank == 0 ? prot->SetA(set0) : prot->GetA(set0.size()));
  auto share1 = (rank == 1 ? prot->SetA(set1) : prot->GetA(set1.size()));
  auto secret = (rank == 1 ? prot->SetA(val1) : prot->GetA(val1.size()));
  SPDLOG_INFO("[P{}] Then, executing Circuit-PSI, set0 {} && set1 {}", rank,
              share0.size(), share1.size());

  // auto result_s = prot->CPSI(share0, share1, secret);
  // same as CPSI
  auto shuffle0 = prot->ShuffleA(share0);
  auto [shuffle1, shuffle_data] = prot->ShuffleA(share1, secret);

  std::vector<size_t> indexes;

  if (fairness) {
    TIMER_START(a2g);  // start a2g_timer
    auto [scalar_a, bits] = prot->RandFairA(1);
    auto reveal0 = prot->ScalarA2G(scalar_a[0], shuffle0);
    auto reveal1 = prot->A2G(shuffle1);
    auto scalar_p = prot->FairA2P(scalar_a, bits);
    auto scalar_mp = ym::MPInt(scalar_p[0].GetVal());
    for (size_t i = 0; i < reveal1.size(); ++i) {
      Ggroup->MulInplace(&reveal1[i], scalar_mp);
    }
    TIMER_END(a2g);    // stop a2g_timer
    TIMER_PRINT(a2g);  // print info

    std::unordered_set<GTy, decltype(group_hash), decltype(group_equal)> lhs(
        reveal0.begin(), reveal0.end(), 2, group_hash, group_equal);

    for (size_t i = 0; i < reveal1.size(); ++i) {
      if (lhs.count(reveal1[i])) {
        indexes.emplace_back(i);
      }
    }
  } else {
    // reveal G-share
    TIMER_START(a2g);  // start a2g_timer
    auto reveal0 = prot->A2G(shuffle0);
    auto reveal1 = prot->A2G(shuffle1);
    TIMER_END(a2g);    // stop a2g_timer
    TIMER_PRINT(a2g);  // print info

    std::unordered_set<GTy, decltype(group_hash), decltype(group_equal)> lhs(
        reveal0.begin(), reveal0.end(), 2, group_hash, group_equal);

    for (size_t i = 0; i < reveal1.size(); ++i) {
      if (lhs.count(reveal1[i])) {
        indexes.emplace_back(i);
      }
    }
  }
  auto result_s = prot->FilterA(absl::MakeConstSpan(shuffle_data),
                                absl::MakeConstSpan(indexes));

  SPDLOG_INFO("[P{}] interset size {}", rank, result_s.size());
  auto sum_s = prot->SumA(result_s);
  auto result_p = prot->A2P(sum_s);

  typedef decltype(std::declval<internal::PTy>().GetVal()) INTEGER;
  auto ret = std::vector<INTEGER>(1);
  ret[0] = result_p[0].GetVal();
  SPDLOG_INFO("[P{}] sum is {}", rank, ret[0]);
  return ret;
}

struct ArgPack {
  uint32_t size0;
  uint32_t size1;
  uint32_t interset_size;
  uint32_t offline;
  uint32_t cache;
  uint32_t fairness;
  uint128_t seed;

  bool operator==(const ArgPack& other) const {
    return (size0 == other.size0) && (size1 == other.size1) &&
           (interset_size == other.interset_size) &&
           (offline == other.offline) && (cache == other.cache);
  }

  bool operator!=(const ArgPack& other) const { return !(*this == other); }
};

bool SyncTask(const std::shared_ptr<yacl::link::Context>& lctx, uint32_t size0,
              uint32_t size1, uint32_t interset_size, uint32_t offline,
              uint32_t cache, uint32_t fairness, uint128_t& seed) {
  uint128_t tmp_seed = yacl::crypto::SecureRandU128();
  ArgPack tmp = {size0, size1,    interset_size, offline,
                 cache, fairness, tmp_seed};
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

std::shared_ptr<yacl::link::Context> MakeLink(const std::string& parties,
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

int main(int argc, char** argv) {
  llvm::cl::ParseCommandLineOptions(argc, argv);

  bool mem_mode = cl_mode.getValue() == 0;
  bool offline = cl_offline.getValue();
  bool cache = cl_cache.getValue();
  bool fairness = cl_fairness.getValue();

  size_t size0 = cl_size0.getValue();
  size_t size1 = cl_size1.getValue();
  size_t interset_size = cl_interset_size.getValue();

  // auto re-correcting
  if (interset_size > std::min(size0, size1)) {
    std::cout << std::endl
              << "[Warning] interset size (" << interset_size
              << ") is greater than the size of size0 (" << size0
              << ") / size1 (" << size1 << ")" << std::endl;
    interset_size = std::min(size0, size1);
    std::cout << "[Recorrect] set inserset size as " << interset_size
              << std::endl;
  }

  std::cout << "Current Task --> P0 with size ( " << size0
            << " ) && P1 with size ( " << size1
            << " ) --> respect size: " << interset_size << std::endl;

  if (mem_mode == true) {
    // execute malicious circuit PSI (and sum the result)

    auto interset = OP::Rand(interset_size);
    auto key0 = OP::Rand(size0);
    auto key1 = OP::Rand(size1);
    auto data = OP::Ones(size1);

    memcpy(key0.data(), interset.data(), interset.size() * sizeof(PTy));
    memcpy(key1.data(), interset.data(), interset.size() * sizeof(PTy));

    auto lctxs = SetupWorld(2);
    auto task0 = std::async([&] {
      return mc_psi(lctxs[0], absl::MakeSpan(key0), absl::MakeSpan(key1),
                    absl::MakeSpan(data), offline == 0, cache, fairness);
    });
    auto task1 = std::async([&] {
      return mc_psi(lctxs[1], absl::MakeSpan(key0), absl::MakeSpan(key1),
                    absl::MakeSpan(data), offline == 0, cache, fairness);
    });
    auto result0 = task0.get();
    auto result1 = task1.get();
    std::cout << "P0 result (sum): " << result0[0] << std::endl;
    std::cout << "P1 result (sum): " << result1[0] << std::endl;
  } else {
    auto lctx = MakeLink(cl_parties.getValue(), cl_rank.getValue());
    uint128_t seed = 0;
    YACL_ENFORCE(SyncTask(lctx, size0, size1, interset_size, offline, cache,
                          fairness, seed));
    auto interset = OP::Rand(seed, interset_size);
    auto key0 = OP::Rand(size0);
    auto key1 = OP::Rand(size1);
    auto data = OP::Ones(size1);

    memcpy(key0.data(), interset.data(), interset.size() * sizeof(PTy));
    memcpy(key1.data(), interset.data(), interset.size() * sizeof(PTy));

    auto res = mc_psi(lctx, absl::MakeSpan(key0), absl::MakeSpan(key1),
                      absl::MakeSpan(data), offline == 0, cache, fairness);
    std::cout << "P" << cl_rank.getValue() << " result (sum): " << res[0]
              << std::endl;
  }

  return 0;
}
