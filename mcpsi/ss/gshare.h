#pragma once
#include <vector>

#include "mcpsi/context/context.h"
#include "mcpsi/ss/ashare.h"
#include "mcpsi/ss/public.h"
#include "mcpsi/ss/type.h"
#include "mcpsi/utils/field.h"
#include "mcpsi/utils/vec_op.h"

namespace mcpsi::internal {

// core
std::vector<MTy> A2M(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in);
std::vector<MTy> A2M_cache(std::shared_ptr<Context>& ctx,
                           absl::Span<const ATy> in);

std::vector<MTy> ScalarA2M(std::shared_ptr<Context>& ctx, const ATy& scalar,
                           absl::Span<const ATy> in);

std::vector<MTy> ScalarA2M_cache(std::shared_ptr<Context>& ctx,
                                 const ATy& scalar, absl::Span<const ATy> in);

// core
std::vector<GTy> M2G(std::shared_ptr<Context>& ctx, absl::Span<const MTy> in);
std::vector<GTy> M2G_cache(std::shared_ptr<Context>& ctx,
                           absl::Span<const MTy> in);

// [Warning] low efficiency !!!
std::vector<GTy> A2G(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in);
std::vector<GTy> A2G_cache(std::shared_ptr<Context>& ctx,
                           absl::Span<const ATy> in);

std::vector<GTy> ScalarA2G(std::shared_ptr<Context>& ctx, const ATy& scalar,
                           absl::Span<const ATy> in);
std::vector<GTy> ScalarA2G_cache(std::shared_ptr<Context>& ctx,
                                 const ATy& scalar, absl::Span<const ATy> in);

// std::vector<GTy> P2G(std::shared_ptr<Context>& ctx, absl::Span<const PTy>
// in);

std::vector<ATy> CPSI(std::shared_ptr<Context>& ctx, absl::Span<const ATy> set0,
                      absl::Span<const ATy> set1, absl::Span<const ATy> data);
std::vector<ATy> CPSI_cache(std::shared_ptr<Context>& ctx,
                            absl::Span<const ATy> set0,
                            absl::Span<const ATy> set1,
                            absl::Span<const ATy> data);

std::vector<ATy> FairCPSI(std::shared_ptr<Context>& ctx,
                          absl::Span<const ATy> set0,
                          absl::Span<const ATy> set1,
                          absl::Span<const ATy> data);
std::vector<ATy> FairCPSI_cache(std::shared_ptr<Context>& ctx,
                                absl::Span<const ATy> set0,
                                absl::Span<const ATy> set1,
                                absl::Span<const ATy> data);
}  // namespace mcpsi::internal
