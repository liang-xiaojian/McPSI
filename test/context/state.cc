#include "test/context/state.h"

namespace test {
// register string
const std::string Prg::id = std::string("Prg");
// register string
const std::string Connection::id = std::string("Connection");

// FIX ME: insecure, commentment should be hash( seed || rand )
uint128_t Connection::SyncSeed() {
  auto seed = yacl::crypto::RandU128(true);
  auto local_seed_bv = yacl::ByteContainerView(&seed, sizeof(seed));
  auto local_hash = yacl::crypto::Sm3(local_seed_bv);
  auto local_hash_bv =
      yacl::ByteContainerView(local_hash.data(), local_hash.size());
  if (rank_ == 0) {
    SendAsync(NextRank(), local_hash_bv, "Commit:R0");
    auto remote_hash_bv = Recv(NextRank(), "Commit:R1");

    SendAsync(NextRank(), local_seed_bv, "Seed:R0");
    auto remote_seed_bv = Recv(NextRank(), "Seed:R1");

    auto remote_hash = yacl::crypto::Sm3(remote_seed_bv);
    auto bv = yacl::ByteContainerView(remote_hash.data(), remote_hash.size());

    YACL_ENFORCE(yacl::ByteContainerView(remote_hash_bv) == bv);
    seed ^= *reinterpret_cast<uint128_t*>(remote_seed_bv.data());
  } else {
    auto remote_hash_bv = Recv(NextRank(), "Commit:R0");
    SendAsync(NextRank(), local_hash_bv, "Commit:R1");

    auto remote_seed_bv = Recv(NextRank(), "Seed:R0");
    SendAsync(NextRank(), local_seed_bv, "Seed:R1");

    auto remote_hash = yacl::crypto::Sm3(remote_seed_bv);
    auto bv = yacl::ByteContainerView(remote_hash.data(), remote_hash.size());

    YACL_ENFORCE(yacl::ByteContainerView(remote_hash_bv) == bv);
    seed ^= *reinterpret_cast<uint128_t*>(remote_seed_bv.data());
  }

  return seed;
}
}  // namespace test
