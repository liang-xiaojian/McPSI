#pragma once
#include <vector>

#include "mcpsi/context/context.h"
#include "mcpsi/ss/ashare.h"
#include "mcpsi/ss/public.h"
#include "mcpsi/ss/type.h"
#include "mcpsi/utils/field.h"
#include "mcpsi/utils/vec_op.h"

namespace test::internal {

// core
std::vector<MTy> A2M(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in);
// core
std::vector<GTy> M2G(std::shared_ptr<Context>& ctx, absl::Span<const MTy> in);

// [Warning] low efficiency !!!
// FIX ME: too much Pow Operation
// Maybe we should use "GMP" instead ???
// trival, since A2G = M2G( A2M )
std::vector<GTy> A2G(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in);

// std::vector<GTy> P2G(std::shared_ptr<Context>& ctx, absl::Span<const PTy>
// in);

}  // namespace test::internal