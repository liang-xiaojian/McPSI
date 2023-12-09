#include "mcpsi/ss/public.h"

#include <vector>

#include "mcpsi/utils/field.h"
#include "mcpsi/utils/vec_op.h"

namespace mcpsi::internal {

std::vector<PTy> AddPP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const PTy> lhs, absl::Span<const PTy> rhs) {
  return op::Add(absl::MakeSpan(lhs), absl::MakeSpan(rhs));
}

std::vector<PTy> SubPP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const PTy> lhs, absl::Span<const PTy> rhs) {
  return op::Sub(absl::MakeSpan(lhs), absl::MakeSpan(rhs));
}

std::vector<PTy> MulPP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const PTy> lhs, absl::Span<const PTy> rhs) {
  return op::Mul(absl::MakeSpan(lhs), absl::MakeSpan(rhs));
}

std::vector<PTy> DivPP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const PTy> lhs, absl::Span<const PTy> rhs) {
  return op::Div(absl::MakeSpan(lhs), absl::MakeSpan(rhs));
}

std::vector<PTy> NegP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                      absl::Span<const PTy> in) {
  return op::Neg(absl::MakeSpan(in));
}

std::vector<PTy> InvP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                      absl::Span<const PTy> in) {
  return op::Inv(absl::MakeSpan(in));
}

std::vector<PTy> OnesP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       size_t num) {
  return op::Ones(num);
}

std::vector<PTy> ZerosP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                        size_t num) {
  return op::Zeros(num);
}

std::vector<PTy> RandP(std::shared_ptr<Context>& ctx, size_t num) {
  return op::Rand(*ctx->GetState<Prg>(), num);
}

}  // namespace mcpsi::internal
