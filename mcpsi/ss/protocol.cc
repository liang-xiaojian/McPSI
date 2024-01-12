#include "mcpsi/ss/protocol.h"

#include "protocol.h"

namespace mcpsi {

// register string
const std::string Protocol::id = std::string("Protocol");

#define RegPP(name)                                            \
  std::vector<PTy> Protocol::name(absl::Span<const PTy> lhs,   \
                                  absl::Span<const PTy> rhs) { \
    return internal::name##PP(ctx_, lhs, rhs);                 \
  }

#define RegAP(name)                                            \
  std::vector<ATy> Protocol::name(absl::Span<const ATy> lhs,   \
                                  absl::Span<const PTy> rhs) { \
    return internal::name##AP(ctx_, lhs, rhs);                 \
  }

#define RegPA(name)                                            \
  std::vector<ATy> Protocol::name(absl::Span<const PTy> lhs,   \
                                  absl::Span<const ATy> rhs) { \
    return internal::name##PA(ctx_, lhs, rhs);                 \
  }

#define RegAA(name)                                            \
  std::vector<ATy> Protocol::name(absl::Span<const ATy> lhs,   \
                                  absl::Span<const ATy> rhs) { \
    return internal::name##AA(ctx_, lhs, rhs);                 \
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

#define RegP(name)                                            \
  std::vector<PTy> Protocol::name(absl::Span<const PTy> in) { \
    return internal::name##P(ctx_, in);                       \
  }

#define RegA(name)                                            \
  std::vector<ATy> Protocol::name(absl::Span<const ATy> in) { \
    return internal::name##A(ctx_, in);                       \
  }

#define RegSi(name) \
  RegP(name);       \
  RegA(name)

RegSi(Neg);
RegSi(Inv);

#define RegConvert(FROM, TO)                                                 \
  std::vector<TO##Ty> Protocol::FROM##2##TO(absl::Span<const FROM##Ty> in) { \
    return internal::FROM##2##TO(ctx_, in);                                  \
  }

RegConvert(A, P);
RegConvert(P, A);
RegConvert(A, M);
RegConvert(M, G);
RegConvert(A, G);

std::vector<PTy> Protocol::ZerosP(size_t num) {
  return internal::ZerosP(ctx_, num);
}

std::vector<PTy> Protocol::RandP(size_t num) {
  return internal::RandP(ctx_, num);
}

std::vector<ATy> Protocol::ZerosA(size_t num) {
  return internal::ZerosA(ctx_, num);
}

std::vector<ATy> Protocol::RandA(size_t num) {
  return internal::RandA(ctx_, num);
}

std::vector<ATy> Protocol::SetA(absl::Span<const PTy> in) {
  return internal::SetA(ctx_, in);
}

std::vector<ATy> Protocol::GetA(size_t num) {
  return internal::GetA(ctx_, num);
}

std::vector<ATy> Protocol::SumA(absl::Span<const ATy> in) {
  return internal::SumA(ctx_, in);
}

std::vector<ATy> Protocol::FilterA(absl::Span<const ATy> in,
                                   absl::Span<const size_t> indexes) {
  return internal::FilterA(ctx_, in, indexes);
}

// shuffle
std::vector<ATy> Protocol::ShuffleA(absl::Span<const ATy> in) {
  return internal::ShuffleA(ctx_, in);
}

std::vector<ATy> Protocol::ShuffleASet(absl::Span<const ATy> in) {
  return internal::ShuffleASet(ctx_, in);
}
std::vector<ATy> Protocol::ShuffleAGet(absl::Span<const ATy> in) {
  return internal::ShuffleAGet(ctx_, in);
}

std::array<std::vector<ATy>, 2> Protocol::ShuffleA(absl::Span<const ATy> in0,
                                                   absl::Span<const ATy> in1) {
  return internal::ShuffleA(ctx_, in0, in1);
}

std::array<std::vector<ATy>, 2> Protocol::ShuffleASet(
    absl::Span<const ATy> in0, absl::Span<const ATy> in1) {
  return internal::ShuffleASet(ctx_, in0, in1);
}

std::array<std::vector<ATy>, 2> Protocol::ShuffleAGet(
    absl::Span<const ATy> in0, absl::Span<const ATy> in1) {
  return internal::ShuffleAGet(ctx_, in0, in1);
}

std::vector<ATy> Protocol::CPSI(absl::Span<const ATy> set0,
                                absl::Span<const ATy> set1,
                                absl::Span<const ATy> data) {
  return internal::CPSI(ctx_, set0, set1, data);
}

}  // namespace mcpsi
