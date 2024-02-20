#pragma once
#include <vector>

#include "mcpsi/context/context.h"
#include "mcpsi/ss/type.h"
#include "mcpsi/utils/field.h"

namespace mcpsi::internal {

std::vector<PTy> AddPP(std::shared_ptr<Context>& ctx, absl::Span<const PTy> lhs,
                       absl::Span<const PTy> rhs);
std::vector<PTy> AddPP_cache(std::shared_ptr<Context>& ctx,
                             absl::Span<const PTy> lhs,
                             absl::Span<const PTy> rhs);

std::vector<PTy> SubPP(std::shared_ptr<Context>& ctx, absl::Span<const PTy> lhs,
                       absl::Span<const PTy> rhs);
std::vector<PTy> SubPP_cache(std::shared_ptr<Context>& ctx,
                             absl::Span<const PTy> lhs,
                             absl::Span<const PTy> rhs);

std::vector<PTy> MulPP(std::shared_ptr<Context>& ctx, absl::Span<const PTy> lhs,
                       absl::Span<const PTy> rhs);
std::vector<PTy> MulPP_cache(std::shared_ptr<Context>& ctx,
                             absl::Span<const PTy> lhs,
                             absl::Span<const PTy> rhs);

std::vector<PTy> DivPP(std::shared_ptr<Context>& ctx, absl::Span<const PTy> lhs,
                       absl::Span<const PTy> rhs);
std::vector<PTy> DivPP_cache(std::shared_ptr<Context>& ctx,
                             absl::Span<const PTy> lhs,
                             absl::Span<const PTy> rhs);

std::vector<PTy> NegP(std::shared_ptr<Context>& ctx, absl::Span<const PTy> in);
std::vector<PTy> NegP_cache(std::shared_ptr<Context>& ctx,
                            absl::Span<const PTy> in);

std::vector<PTy> InvP(std::shared_ptr<Context>& ctx, absl::Span<const PTy> in);
std::vector<PTy> InvP_cache(std::shared_ptr<Context>& ctx,
                            absl::Span<const PTy> in);

std::vector<PTy> OnesP(std::shared_ptr<Context>& ctx, size_t num);
std::vector<PTy> OnesP_cache(std::shared_ptr<Context>& ctx, size_t num);

std::vector<PTy> ZerosP(std::shared_ptr<Context>& ctx, size_t num);
std::vector<PTy> ZerosP_cache(std::shared_ptr<Context>& ctx, size_t num);

std::vector<PTy> RandP(std::shared_ptr<Context>& ctx, size_t num);
std::vector<PTy> RandP_cache(std::shared_ptr<Context>& ctx, size_t num);

}  // namespace mcpsi::internal
