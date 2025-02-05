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

#define RegWithParam1(name, param0)              \
  if (cache) {                                   \
    return internal::name##_cache(ctx_, param0); \
  }                                              \
  return internal::name(ctx_, param0)

#define RegWithParam2(name, param0, param1)              \
  if (cache) {                                           \
    return internal::name##_cache(ctx_, param0, param1); \
  }                                                      \
  return internal::name(ctx_, param0, param1)

#define RegWithParam3(name, param0, param1, param2)              \
  if (cache) {                                                   \
    return internal::name##_cache(ctx_, param0, param1, param2); \
  }                                                              \
  return internal::name(ctx_, param0, param1, param2)

std::vector<PTy> Protocol::ZerosP(size_t num, bool cache) {
  RegWithParam1(ZerosP, num);
}

std::vector<PTy> Protocol::RandP(size_t num, bool cache) {
  RegWithParam1(RandP, num);
}

std::vector<ATy> Protocol::ZerosA(size_t num, bool cache) {
  RegWithParam1(ZerosA, num);
}

std::vector<ATy> Protocol::RandA(size_t num, bool cache) {
  RegWithParam1(RandA, num);
}

std::vector<ATy> Protocol::SetA(absl::Span<const PTy> in, bool cache) {
  RegWithParam1(SetA, in);
}

std::vector<ATy> Protocol::GetA(size_t num, bool cache) {
  RegWithParam1(GetA, num);
}

std::vector<ATy> Protocol::SumA(absl::Span<const ATy> in, bool cache) {
  RegWithParam1(SumA, in);
}

std::vector<ATy> Protocol::FilterA(absl::Span<const ATy> in,
                                   absl::Span<const size_t> indexes,
                                   bool cache) {
  RegWithParam2(FilterA, in, indexes);
}

// shuffle
std::vector<ATy> Protocol::ShuffleA(absl::Span<const ATy> in, bool cache) {
  RegWithParam1(ShuffleA, in);
}

std::vector<ATy> Protocol::ShuffleASet(absl::Span<const ATy> in, bool cache) {
  RegWithParam1(ShuffleASet, in);
}
std::vector<ATy> Protocol::ShuffleAGet(absl::Span<const ATy> in, bool cache) {
  RegWithParam1(ShuffleAGet, in);
}

std::array<std::vector<ATy>, 2> Protocol::ShuffleA(absl::Span<const ATy> in0,
                                                   absl::Span<const ATy> in1,
                                                   bool cache) {
  RegWithParam2(ShuffleA, in0, in1);
}

std::array<std::vector<ATy>, 2> Protocol::ShuffleASet(absl::Span<const ATy> in0,
                                                      absl::Span<const ATy> in1,
                                                      bool cache) {
  RegWithParam2(ShuffleASet, in0, in1);
}

std::array<std::vector<ATy>, 2> Protocol::ShuffleAGet(absl::Span<const ATy> in0,
                                                      absl::Span<const ATy> in1,
                                                      bool cache) {
  RegWithParam2(ShuffleAGet, in0, in1);
}

std::vector<ATy> Protocol::ZeroOneA(size_t num, bool cache) {
  RegWithParam1(ZeroOneA, num);
}

std::vector<ATy> Protocol::ScalarMulPA(const PTy& scalar,
                                       absl::Span<const ATy> in, bool cache) {
  RegWithParam2(ScalarMulPA, scalar, in);
}

std::vector<ATy> Protocol::ScalarMulAP(const ATy& scalar,
                                       absl::Span<const PTy> in, bool cache) {
  RegWithParam2(ScalarMulAP, scalar, in);
}

std::pair<std::vector<ATy>, std::vector<ATy>> Protocol::RandFairA(size_t num,
                                                                  bool cache) {
  RegWithParam1(RandFairA, num);
}

std::vector<PTy> Protocol::FairA2P(absl::Span<const ATy> in,
                                   absl::Span<const ATy> bits, bool cache) {
  RegWithParam2(FairA2P, in, bits);
}

std::vector<ATy> Protocol::DyExp(absl::Span<const ATy> in, bool cache) {
  RegWithParam1(DyExp, in);
}

std::vector<ATy> Protocol::DyExpSet(absl::Span<const PTy> in, bool cache) {
  RegWithParam1(DyExpSet, in);
}

std::vector<ATy> Protocol::DyExpGet(size_t num, bool cache) {
  RegWithParam1(DyExpGet, num);
}

std::vector<ATy> Protocol::ScalarDyExp(const ATy& scalar,
                                       absl::Span<const ATy> in, bool cache) {
  RegWithParam2(ScalarDyExp, scalar, in);
}

std::vector<ATy> Protocol::ScalarDyExpSet(const ATy& scalar,
                                          absl::Span<const PTy> in,
                                          bool cache) {
  RegWithParam2(ScalarDyExpSet, scalar, in);
}

std::vector<ATy> Protocol::ScalarDyExpGet(const ATy& scalar, size_t num,
                                          bool cache) {
  RegWithParam2(ScalarDyExpGet, scalar, num);
}

std::vector<GTy> Protocol::DyOprf(absl::Span<const ATy> in, bool cache) {
  RegWithParam1(DyOprf, in);
}

std::vector<GTy> Protocol::DyOprfSet(absl::Span<const PTy> in, bool cache) {
  RegWithParam1(DyOprfSet, in);
}

std::vector<GTy> Protocol::DyOprfGet(size_t num, bool cache) {
  RegWithParam1(DyOprfGet, num);
}

std::vector<GTy> Protocol::ScalarDyOprf(const ATy& scalar,
                                        absl::Span<const ATy> in, bool cache) {
  RegWithParam2(ScalarDyOprf, scalar, in);
}

std::vector<GTy> Protocol::ScalarDyOprfSet(const ATy& scalar,
                                           absl::Span<const PTy> in,
                                           bool cache) {
  RegWithParam2(ScalarDyOprfSet, scalar, in);
}

std::vector<GTy> Protocol::ScalarDyOprfGet(const ATy& scalar, size_t num,
                                           bool cache) {
  RegWithParam2(ScalarDyOprfGet, scalar, num);
}

std::vector<ATy> Protocol::CPSI(absl::Span<const ATy> set0,
                                absl::Span<const ATy> set1,
                                absl::Span<const ATy> data, bool cache) {
  RegWithParam3(CPSI, set0, set1, data);
}

std::vector<ATy> Protocol::FairCPSI(absl::Span<const ATy> set0,
                                    absl::Span<const ATy> set1,
                                    absl::Span<const ATy> data, bool cache) {
  RegWithParam3(FairCPSI, set0, set1, data);
}

}  // namespace mcpsi
