#include "mcpsi/ss/protocol.h"

#include "protocol.h"

namespace mcpsi {

// register string
const std::string Protocol::id = std::string("Protocol");

#define RegPP(name)                                                        \
  std::vector<PTy> Protocol::name(absl::Span<const PTy> lhs,               \
                                  absl::Span<const PTy> rhs, bool cache) { \
    if (cache) {                                                           \
      return internal::name##PP_cache(ctx_, lhs, rhs);                     \
    }                                                                      \
    return internal::name##PP(ctx_, lhs, rhs);                             \
  }

#define RegAP(name)                                                        \
  std::vector<ATy> Protocol::name(absl::Span<const ATy> lhs,               \
                                  absl::Span<const PTy> rhs, bool cache) { \
    if (cache) {                                                           \
      return internal::name##AP_cache(ctx_, lhs, rhs);                     \
    }                                                                      \
    return internal::name##AP(ctx_, lhs, rhs);                             \
  }

#define RegPA(name)                                                        \
  std::vector<ATy> Protocol::name(absl::Span<const PTy> lhs,               \
                                  absl::Span<const ATy> rhs, bool cache) { \
    if (cache) {                                                           \
      return internal::name##PA_cache(ctx_, lhs, rhs);                     \
    }                                                                      \
    return internal::name##PA(ctx_, lhs, rhs);                             \
  }

#define RegAA(name)                                                        \
  std::vector<ATy> Protocol::name(absl::Span<const ATy> lhs,               \
                                  absl::Span<const ATy> rhs, bool cache) { \
    if (cache) {                                                           \
      return internal::name##AA_cache(ctx_, lhs, rhs);                     \
    }                                                                      \
    return internal::name##AA(ctx_, lhs, rhs);                             \
  }

#define RegBi(name) \
  RegAA(name);      \
  RegAP(name);      \
  RegPA(name);      \
  RegPP(name)

RegBi(Add);
RegBi(Sub);
RegBi(Mul);
RegBi(Div);

#define RegP(name)                                                        \
  std::vector<PTy> Protocol::name(absl::Span<const PTy> in, bool cache) { \
    if (cache) {                                                          \
      return internal::name##P_cache(ctx_, in);                           \
    }                                                                     \
    return internal::name##P(ctx_, in);                                   \
  }

#define RegA(name)                                                        \
  std::vector<ATy> Protocol::name(absl::Span<const ATy> in, bool cache) { \
    if (cache) {                                                          \
      return internal::name##A_cache(ctx_, in);                           \
    }                                                                     \
    return internal::name##A(ctx_, in);                                   \
  }

#define RegSi(name) \
  RegP(name);       \
  RegA(name)

RegSi(Neg);
RegSi(Inv);

#define RegConvert(FROM, TO)                                               \
  std::vector<TO##Ty> Protocol::FROM##2##TO(absl::Span<const FROM##Ty> in, \
                                            bool cache) {                  \
    if (cache) {                                                           \
      return internal::FROM##2##TO##_cache(ctx_, in);                      \
    }                                                                      \
    return internal::FROM##2##TO(ctx_, in);                                \
  }

RegConvert(A, P);
RegConvert(P, A);
RegConvert(A, M);
RegConvert(M, G);
RegConvert(A, G);

#define DispatchAll(name, ...)                        \
  if (cache) {                                        \
    return internal::name##_cache(ctx_, __VA_ARGS__); \
  }                                                   \
  return internal::name(ctx_, __VA_ARGS__)

std::vector<PTy> Protocol::ZerosP(size_t num, bool cache) {
  DispatchAll(ZerosP, num);
}

std::vector<PTy> Protocol::RandP(size_t num, bool cache) {
  DispatchAll(RandP, num);
}

std::vector<ATy> Protocol::ZerosA(size_t num, bool cache) {
  DispatchAll(ZerosA, num);
}

std::vector<ATy> Protocol::RandA(size_t num, bool cache) {
  DispatchAll(RandA, num);
}

std::vector<ATy> Protocol::SetA(absl::Span<const PTy> in, bool cache) {
  DispatchAll(SetA, in);
}

std::vector<ATy> Protocol::GetA(size_t num, bool cache) {
  DispatchAll(GetA, num);
}

std::vector<ATy> Protocol::SumA(absl::Span<const ATy> in, bool cache) {
  DispatchAll(SumA, in);
}

std::vector<ATy> Protocol::FilterA(absl::Span<const ATy> in,
                                   absl::Span<const size_t> indexes,
                                   bool cache) {
  DispatchAll(FilterA, in, indexes);
}

// shuffle
std::vector<ATy> Protocol::ShuffleA(absl::Span<const ATy> in, bool cache) {
  DispatchAll(ShuffleA, in);
}

std::vector<ATy> Protocol::ShuffleASet(absl::Span<const ATy> in, bool cache) {
  DispatchAll(ShuffleASet, in);
}
std::vector<ATy> Protocol::ShuffleAGet(absl::Span<const ATy> in, bool cache) {
  DispatchAll(ShuffleAGet, in);
}

std::array<std::vector<ATy>, 2> Protocol::ShuffleA(absl::Span<const ATy> in0,
                                                   absl::Span<const ATy> in1,
                                                   bool cache) {
  DispatchAll(ShuffleA, in0, in1);
}

std::array<std::vector<ATy>, 2> Protocol::ShuffleASet(absl::Span<const ATy> in0,
                                                      absl::Span<const ATy> in1,
                                                      bool cache) {
  DispatchAll(ShuffleASet, in0, in1);
}

std::array<std::vector<ATy>, 2> Protocol::ShuffleAGet(absl::Span<const ATy> in0,
                                                      absl::Span<const ATy> in1,
                                                      bool cache) {
  DispatchAll(ShuffleAGet, in0, in1);
}

std::vector<ATy> Protocol::ZeroOneA(size_t num, bool cache) {
  DispatchAll(ZeroOneA, num);
}

std::vector<ATy> Protocol::ScalarMulPA(const PTy& scalar,
                                       absl::Span<const ATy> in, bool cache) {
  DispatchAll(ScalarMulPA, scalar, in);
}

std::vector<ATy> Protocol::ScalarMulAP(const ATy& scalar,
                                       absl::Span<const PTy> in, bool cache) {
  DispatchAll(ScalarMulAP, scalar, in);
}

std::pair<std::vector<ATy>, std::vector<ATy>> Protocol::RandFairA(size_t num,
                                                                  bool cache) {
  DispatchAll(RandFairA, num);
}

std::vector<PTy> Protocol::FairA2P(absl::Span<const ATy> in,
                                   absl::Span<const ATy> bits, bool cache) {
  DispatchAll(FairA2P, in, bits);
}

std::vector<ATy> Protocol::DyExp(absl::Span<const ATy> in, bool cache) {
  DispatchAll(DyExp, in);
}

std::vector<ATy> Protocol::DyExpSet(absl::Span<const PTy> in, bool cache) {
  DispatchAll(DyExpSet, in);
}

std::vector<ATy> Protocol::DyExpGet(size_t num, bool cache) {
  DispatchAll(DyExpGet, num);
}

std::vector<ATy> Protocol::ScalarDyExp(const ATy& scalar,
                                       absl::Span<const ATy> in, bool cache) {
  DispatchAll(ScalarDyExp, scalar, in);
}

std::vector<ATy> Protocol::ScalarDyExpSet(const ATy& scalar,
                                          absl::Span<const PTy> in,
                                          bool cache) {
  DispatchAll(ScalarDyExpSet, scalar, in);
}

std::vector<ATy> Protocol::ScalarDyExpGet(const ATy& scalar, size_t num,
                                          bool cache) {
  DispatchAll(ScalarDyExpGet, scalar, num);
}

std::vector<GTy> Protocol::DyOprf(absl::Span<const ATy> in, bool cache) {
  DispatchAll(DyOprf, in);
}

std::vector<GTy> Protocol::DyOprfSet(absl::Span<const PTy> in, bool cache) {
  DispatchAll(DyOprfSet, in);
}

std::vector<GTy> Protocol::DyOprfGet(size_t num, bool cache) {
  DispatchAll(DyOprfGet, num);
}

std::vector<GTy> Protocol::ScalarDyOprf(const ATy& scalar,
                                        absl::Span<const ATy> in, bool cache) {
  DispatchAll(ScalarDyOprf, scalar, in);
}

std::vector<GTy> Protocol::ScalarDyOprfSet(const ATy& scalar,
                                           absl::Span<const PTy> in,
                                           bool cache) {
  DispatchAll(ScalarDyOprfSet, scalar, in);
}

std::vector<GTy> Protocol::ScalarDyOprfGet(const ATy& scalar, size_t num,
                                           bool cache) {
  DispatchAll(ScalarDyOprfGet, scalar, num);
}

std::vector<ATy> Protocol::CPSI(absl::Span<const ATy> set0,
                                absl::Span<const ATy> set1,
                                absl::Span<const ATy> data, bool cache) {
  DispatchAll(CPSI, set0, set1, data);
}

std::vector<ATy> Protocol::FairCPSI(absl::Span<const ATy> set0,
                                    absl::Span<const ATy> set1,
                                    absl::Span<const ATy> data, bool cache) {
  DispatchAll(FairCPSI, set0, set1, data);
}

void Protocol::CheckBufferAppend(absl::Span<const PTy> in) {
  std::copy(in.begin(), in.end(), std::back_inserter(check_buff_));
}

void Protocol::CheckBufferAppend(const PTy& in) {
  check_buff_.emplace_back(in);
}

bool Protocol::DelayCheck() {
  if (check_buff_.size() == 0) {
    return true;
  }
  auto conn = ctx_->GetConnection();

  auto hash_val = yacl::crypto::Sm3(yacl::ByteContainerView(
      check_buff_.data(), check_buff_.size() * sizeof(internal::PTy)));
  check_buff_.clear();

  auto remote_hash_val = conn->ExchangeWithCommit(
      yacl::ByteContainerView(hash_val.data(), hash_val.size()));
  auto flag = (yacl::ByteContainerView(hash_val.data(), hash_val.size()) ==
               yacl::ByteContainerView(remote_hash_val));
  SPDLOG_INFO("delay check result: {} ", flag);
  return flag;
}

}  // namespace mcpsi
