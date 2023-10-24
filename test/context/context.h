#pragma once

#include <memory>

#include "test/context/state.h"
#include "yacl/crypto/base/hash/hash_utils.h"
#include "yacl/crypto/tools/prg.h"
#include "yacl/crypto/utils/rand.h"
#include "yacl/link/link.h"

namespace test {

class Context {
 public:
  std::unique_ptr<StateList> states_{nullptr};

 public:
  Context(std::shared_ptr<yacl::link::Context> lctx) : lctx_(lctx) {
    states_ = std::make_unique<StateList>();
    rank_ = lctx_->Rank();
  }

  uint128_t SyncSeed() {
    auto seed = yacl::crypto::RandU128(true);
    auto local_seed_bv = yacl::ByteContainerView(&seed, sizeof(seed));
    auto local_hash = yacl::crypto::Sm3(local_seed_bv);
    auto local_hash_bv =
        yacl::ByteContainerView(local_hash.data(), local_hash.size());
    if (rank_ == 0) {
      lctx_->SendAsync(NextRank(), local_hash_bv, "Commit:R0");
      auto remote_hash_bv = lctx_->Recv(NextRank(), "Commit:R1");

      lctx_->SendAsync(NextRank(), local_seed_bv, "Seed:R0");
      auto remote_seed_bv = lctx_->Recv(NextRank(), "Seed:R1");

      auto remote_hash = yacl::crypto::Sm3(remote_seed_bv);
      auto bv = yacl::ByteContainerView(remote_hash.data(), remote_hash.size());

      YACL_ENFORCE(yacl::ByteContainerView(remote_hash_bv) == bv);
      seed ^= *reinterpret_cast<uint128_t*>(remote_seed_bv.data());
    } else {
      auto remote_hash_bv = lctx_->Recv(NextRank(), "Commit:R0");
      lctx_->SendAsync(NextRank(), local_hash_bv, "Commit:R1");

      auto remote_seed_bv = lctx_->Recv(NextRank(), "Seed:R0");
      lctx_->SendAsync(NextRank(), local_seed_bv, "Seed:R1");

      auto remote_hash = yacl::crypto::Sm3(remote_seed_bv);
      auto bv = yacl::ByteContainerView(remote_hash.data(), remote_hash.size());

      YACL_ENFORCE(yacl::ByteContainerView(remote_hash_bv) == bv);
      seed ^= *reinterpret_cast<uint128_t*>(remote_seed_bv.data());
    }

    return seed;
  }

  std::shared_ptr<yacl::link::Context> GetLink() { return lctx_; }

  uint32_t NextRank() { return lctx_->NextRank(); }

  uint32_t GetRank() { return rank_; }

  template <typename StateTy>
  std::shared_ptr<StateTy> GetState() {
    return states_->template GetState<StateTy>();
  }

 private:
  std::shared_ptr<yacl::link::Context> lctx_{nullptr};
  uint32_t rank_;
};

};  // namespace test
