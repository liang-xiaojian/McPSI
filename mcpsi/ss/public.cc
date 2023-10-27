#include "mcpsi/ss/public.h"

#include <vector>

#include "mcpsi/utils/field.h"
#include "mcpsi/utils/vec_op.h"

namespace test::internal {

std::vector<PTy> AddPP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const PTy> lhs, absl::Span<const PTy> rhs) {
  return Add(absl::MakeSpan(lhs), absl::MakeSpan(rhs));
}

std::vector<PTy> SubPP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const PTy> lhs, absl::Span<const PTy> rhs) {
  return Sub(absl::MakeSpan(lhs), absl::MakeSpan(rhs));
}

std::vector<PTy> MulPP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const PTy> lhs, absl::Span<const PTy> rhs) {
  return Mul(absl::MakeSpan(lhs), absl::MakeSpan(rhs));
}

std::vector<PTy> DivPP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const PTy> lhs, absl::Span<const PTy> rhs) {
  return Div(absl::MakeSpan(lhs), absl::MakeSpan(rhs));
}

std::vector<PTy> NegP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                      absl::Span<const PTy> in) {
  return Neg(absl::MakeSpan(in));
}

std::vector<PTy> InvP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                      absl::Span<const PTy> in) {
  return Inv(absl::MakeSpan(in));
}

std::vector<PTy> OnesP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       size_t num) {
  return Ones(num);
}

std::vector<PTy> ZerosP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                        size_t num) {
  return Zeros(num);
}

std::vector<PTy> RandP(std::shared_ptr<Context>& ctx, size_t num) {
  return Rand(*ctx->GetState<Prg>(), num);
}

}  // namespace test::internal
