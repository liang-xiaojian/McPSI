#include "test/ss/protocol.h"

namespace test {

// register string
const std::string Protocol::id = std::string("Protocol");

#define RegPP(name)                                            \
  std::vector<PTy> Protocol::name(absl::Span<const PTy> lhs,   \
                                  absl::Span<const PTy> rhs) { \
    return internal::name##PP(ctx_, lhs, rhs);                 \
  }

RegPP(Add);
RegPP(Sub);
RegPP(Mul);
RegPP(Div);

#define RegAP(name)                                            \
  std::vector<ATy> Protocol::name(absl::Span<const ATy> lhs,   \
                                  absl::Span<const PTy> rhs) { \
    return internal::name##AP(ctx_, lhs, rhs);                 \
  }

RegAP(Add);
RegAP(Sub);
RegAP(Mul);
RegAP(Div);

#define RegPA(name)                                            \
  std::vector<ATy> Protocol::name(absl::Span<const PTy> lhs,   \
                                  absl::Span<const ATy> rhs) { \
    return internal::name##PA(ctx_, lhs, rhs);                 \
  }

RegPA(Add);
RegPA(Sub);
RegPA(Mul);
RegPA(Div);

#define RegAA(name)                                            \
  std::vector<ATy> Protocol::name(absl::Span<const ATy> lhs,   \
                                  absl::Span<const ATy> rhs) { \
    return internal::name##AA(ctx_, lhs, rhs);                 \
  }

RegAA(Add);
RegAA(Sub);
RegAA(Mul);
RegAA(Div);

#define RegP(name)                                            \
  std::vector<PTy> Protocol::name(absl::Span<const PTy> in) { \
    return internal::name##P(ctx_, in);                       \
  }

#define RegA(name)                                            \
  std::vector<ATy> Protocol::name(absl::Span<const ATy> in) { \
    return internal::name##A(ctx_, in);                       \
  }

RegP(Neg);
RegP(Inv);
RegA(Neg);
RegA(Inv);

std::vector<PTy> Protocol::A2P(absl::Span<const ATy> in) {
  return internal::A2P(ctx_, in);
}
std::vector<ATy> Protocol::P2A(absl::Span<const PTy> in) {
  return internal::P2A(ctx_, in);
}

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

// shuffle
std::vector<ATy> Protocol::ShuffleA(absl::Span<const ATy> in) {
  return internal::ShuffleA(ctx_, in);
}

std::vector<ATy> Protocol::ShuffleA(absl::Span<const ATy> in,
                                    absl::Span<const size_t> perm) {
  return internal::ShuffleA(ctx_, in, perm);
}

std::vector<ATy> Protocol::ShuffleASet(absl::Span<const ATy> in,
                                       absl::Span<const size_t> perm) {
  return internal::ShuffleASet(ctx_, in, perm);
}
std::vector<ATy> Protocol::ShuffleAGet(absl::Span<const ATy> in) {
  return internal::ShuffleAGet(ctx_, in);
}

}  // namespace test
