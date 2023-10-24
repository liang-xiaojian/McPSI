#pragma once
#include <vector>

#include "test/context/context.h"
#include "test/ss/type.h"
#include "test/utils/field.h"

namespace test::internal {

std::vector<PTy> AddPP(std::shared_ptr<Context>& ctx, absl::Span<const PTy> lhs,
                       absl::Span<const PTy> rhs);

std::vector<PTy> SubPP(std::shared_ptr<Context>& ctx, absl::Span<const PTy> lhs,
                       absl::Span<const PTy> rhs);

std::vector<PTy> MulPP(std::shared_ptr<Context>& ctx, absl::Span<const PTy> lhs,
                       absl::Span<const PTy> rhs);

std::vector<PTy> DivPP(std::shared_ptr<Context>& ctx, absl::Span<const PTy> lhs,
                       absl::Span<const PTy> rhs);

std::vector<PTy> NegP(std::shared_ptr<Context>& ctx, absl::Span<const PTy> in);

std::vector<PTy> InvP(std::shared_ptr<Context>& ctx, absl::Span<const PTy> in);

std::vector<PTy> OnesP(std::shared_ptr<Context>& ctx, size_t num);

std::vector<PTy> ZerosP(std::shared_ptr<Context>& ctx, size_t num);

std::vector<PTy> RandP(std::shared_ptr<Context>& ctx, size_t num);

}  // namespace test::internal
