#include "mcpsi/context/state.h"

namespace mcpsi {
// register string
const std::string Prg::id = std::string("Prg");
// register string
const std::string Connection::id = std::string("Connection");

uint128_t Connection::SyncSeed() {
  auto seed = yacl::crypto::RandU128(true);
  return seed ^ ExchangeWithCommit(seed);
}

template <typename T>
T Connection::_ExchangeWithCommit(T val) {
  // Padding as ( val || randomness )
  yacl::Buffer buff(sizeof(T) + sizeof(uint128_t));
  *reinterpret_cast<T*>(buff.data()) = val;
  *reinterpret_cast<T*>((uint8_t*)buff.data() + sizeof(T)) =
      yacl::crypto::RandU128(true);
  auto buff_bv = yacl::ByteContainerView(buff);

  // commitment = hash( val || randomness )
  auto commitment = yacl::crypto::Sm3(buff);
  auto commitment_bv = yacl::ByteContainerView(commitment);

  T ret;
  if (rank_ == 0) {
    SendAsync(NextRank(), commitment_bv, "Commit:0");
    auto remote_commitment = Recv(NextRank(), "Commit:1");

    SendAsync(NextRank(), buff_bv, "val:0");
    auto remote_buff = Recv(NextRank(), "val:1");

    auto check_commitment = yacl::crypto::Sm3(remote_buff);
    auto check_bv = yacl::ByteContainerView(check_commitment);

    YACL_ENFORCE(yacl::ByteContainerView(remote_commitment) == check_bv);

    ret = *reinterpret_cast<T*>(remote_buff.data());
  } else {
    auto remote_commitment = Recv(NextRank(), "Commit:0");
    SendAsync(NextRank(), commitment_bv, "Commit:1");

    auto remote_buff = Recv(NextRank(), "val:0");
    SendAsync(NextRank(), buff_bv, "val:1");

    auto check_commitment = yacl::crypto::Sm3(remote_buff);
    auto check_bv = yacl::ByteContainerView(check_commitment);

    YACL_ENFORCE(yacl::ByteContainerView(remote_commitment) == check_bv);

    ret = *reinterpret_cast<T*>(remote_buff.data());
  }
  return ret;
}

#define ExchangeWithCommitImpl(T) \
  T Connection::ExchangeWithCommit(T val) { return _ExchangeWithCommit(val); }

ExchangeWithCommitImpl(uint128_t);
ExchangeWithCommitImpl(uint64_t);
}  // namespace mcpsi
