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

yacl::Buffer Connection::_ExchangeWithCommit_Buffer(
    yacl::ByteContainerView bv) {
  yacl::Buffer buff(bv.size() + sizeof(uint128_t));
  memcpy(buff.data(), bv.data(), bv.size());
  *reinterpret_cast<uint128_t*>((uint8_t*)buff.data() + bv.size()) =
      yacl::crypto::RandU128(true);

  auto buff_bv = yacl::ByteContainerView(buff);

  // commitment = hash( val || randomness )
  auto commitment = yacl::crypto::Sm3(buff);
  auto commitment_bv = yacl::ByteContainerView(commitment);

  yacl::Buffer ret;
  if (rank_ == 0) {
    SendAsync(NextRank(), commitment_bv, "Commit:0");
    auto remote_commitment = Recv(NextRank(), "Commit:1");

    SendAsync(NextRank(), buff_bv, "val:0");
    auto remote_buff = Recv(NextRank(), "val:1");

    auto check_commitment = yacl::crypto::Sm3(remote_buff);
    auto check_bv = yacl::ByteContainerView(check_commitment);

    YACL_ENFORCE(yacl::ByteContainerView(remote_commitment) == check_bv);

    ret = std::move(remote_buff);
    ret.resize(bv.size());
  } else {
    auto remote_commitment = Recv(NextRank(), "Commit:0");
    SendAsync(NextRank(), commitment_bv, "Commit:1");

    auto remote_buff = Recv(NextRank(), "val:0");
    SendAsync(NextRank(), buff_bv, "val:1");

    auto check_commitment = yacl::crypto::Sm3(remote_buff);
    auto check_bv = yacl::ByteContainerView(check_commitment);

    YACL_ENFORCE(yacl::ByteContainerView(remote_commitment) == check_bv);

    ret = std::move(remote_buff);
    ret.resize(bv.size());
  }
  return ret;
}

template <typename T>
T Connection::_ExchangeWithCommit_T(T val) {
  // Padding as ( val || randomness )
  yacl::Buffer buff(sizeof(T) + sizeof(uint128_t));
  *reinterpret_cast<T*>(buff.data()) = val;
  *reinterpret_cast<uint128_t*>((uint8_t*)buff.data() + sizeof(T)) =
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
  T Connection::ExchangeWithCommit(T val) { return _ExchangeWithCommit_T(val); }

ExchangeWithCommitImpl(uint128_t);
ExchangeWithCommitImpl(uint64_t);

yacl::Buffer Connection::ExchangeWithCommit(yacl::ByteContainerView bv) {
  return _ExchangeWithCommit_Buffer(bv);
}

}  // namespace mcpsi
