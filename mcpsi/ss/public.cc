#include "mcpsi/ss/public.h"

#include <vector>

#include "mcpsi/utils/field.h"
#include "mcpsi/utils/vec_op.h"

namespace mcpsi::internal {

std::vector<PTy> AddPP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const PTy> lhs, absl::Span<const PTy> rhs) {
  return vec64::Add(absl::MakeSpan(lhs), absl::MakeSpan(rhs));
}

std::vector<PTy> SubPP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const PTy> lhs, absl::Span<const PTy> rhs) {
  return vec64::Sub(absl::MakeSpan(lhs), absl::MakeSpan(rhs));
}

std::vector<PTy> MulPP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const PTy> lhs, absl::Span<const PTy> rhs) {
  return vec64::Mul(absl::MakeSpan(lhs), absl::MakeSpan(rhs));
}

std::vector<PTy> DivPP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const PTy> lhs, absl::Span<const PTy> rhs) {
  return vec64::Div(absl::MakeSpan(lhs), absl::MakeSpan(rhs));
}

std::vector<PTy> NegP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                      absl::Span<const PTy> in) {
  return vec64::Neg(absl::MakeSpan(in));
}

std::vector<PTy> InvP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                      absl::Span<const PTy> in) {
  return vec64::Inv(absl::MakeSpan(in));
}

std::vector<PTy> OnesP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       size_t num) {
  return vec64::Ones(num);
}

std::vector<PTy> ZerosP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                        size_t num) {
  return vec64::Zeros(num);
}

std::vector<PTy> RandP(std::shared_ptr<Context>& ctx, size_t num) {
  return vec64::Rand(*ctx->GetState<Prg>(), num);
}

}  // namespace mcpsi::internal
