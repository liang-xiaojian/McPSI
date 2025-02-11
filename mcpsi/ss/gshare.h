#pragma once
#include <memory>
#include <vector>

#include "mcpsi/context/context.h"
#include "mcpsi/ss/ashare.h"
#include "mcpsi/ss/public.h"
#include "mcpsi/ss/type.h"
#include "mcpsi/utils/field.h"
#include "mcpsi/utils/vec_op.h"

namespace mcpsi::internal {

// DY-exponent
std::vector<ATy> DyExp(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in);
std::vector<ATy> DyExp_cache(std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> in);

std::vector<ATy> DyExpGet(std::shared_ptr<Context>& ctx, size_t num);
std::vector<ATy> DyExpGet_cache(std::shared_ptr<Context>& ctx, size_t num);

std::vector<ATy> DyExpSet(std::shared_ptr<Context>& ctx,
                          absl::Span<const PTy> in);
std::vector<ATy> DyExpSet_cache(std::shared_ptr<Context>& ctx,
                                absl::Span<const PTy> in);

// Fair-DY-exponet
std::vector<ATy> ScalarDyExp(std::shared_ptr<Context>& ctx, const ATy& scalar,
                             absl::Span<const ATy> in);
std::vector<ATy> ScalarDyExp_cache(std::shared_ptr<Context>& ctx,
                                   const ATy& scalar, absl::Span<const ATy> in);

std::vector<ATy> ScalarDyExpGet(std::shared_ptr<Context>& ctx,
                                const ATy& scalar, size_t num);
std::vector<ATy> ScalarDyExpGet_cache(std::shared_ptr<Context>& ctx,
                                      const ATy& scalar, size_t num);

std::vector<ATy> ScalarDyExpSet(std::shared_ptr<Context>& ctx,
                                const ATy& scalar, absl::Span<const PTy> in);
std::vector<ATy> ScalarDyExpSet_cache(std::shared_ptr<Context>& ctx,
                                      const ATy& scalar,
                                      absl::Span<const PTy> in);
// core
std::vector<MTy> A2M(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in);
std::vector<MTy> A2M_cache(std::shared_ptr<Context>& ctx,
                           absl::Span<const ATy> in);

// core
std::vector<GTy> M2G(std::shared_ptr<Context>& ctx, absl::Span<const MTy> in);
std::vector<GTy> M2G_cache(std::shared_ptr<Context>& ctx,
                           absl::Span<const MTy> in);

// A2G = A2M + M2G
std::vector<GTy> A2G(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in);
std::vector<GTy> A2G_cache(std::shared_ptr<Context>& ctx,
                           absl::Span<const ATy> in);

// DY-OPRF = DY-exponent + A2M + M2G
std::vector<GTy> DyOprf(std::shared_ptr<Context>& ctx,
                        absl::Span<const ATy> in);
std::vector<GTy> DyOprf_cache(std::shared_ptr<Context>& ctx,
                              absl::Span<const ATy> in);

std::vector<GTy> DyOprfGet(std::shared_ptr<Context>& ctx, size_t num);
std::vector<GTy> DyOprfGet_cache(std::shared_ptr<Context>& ctx, size_t num);

std::vector<GTy> DyOprfSet(std::shared_ptr<Context>& ctx,
                           absl::Span<const PTy> in);
std::vector<GTy> DyOprfSet_cache(std::shared_ptr<Context>& ctx,
                                 absl::Span<const PTy> in);

std::vector<GTy> ScalarDyOprf(std::shared_ptr<Context>& ctx, const ATy& scalar,
                              absl::Span<const ATy> in);
std::vector<GTy> ScalarDyOprf_cache(std::shared_ptr<Context>& ctx,
                                    const ATy& scalar,
                                    absl::Span<const ATy> in);

std::vector<GTy> ScalarDyOprfGet(std::shared_ptr<Context>& ctx,
                                 const ATy& scalar, size_t num);
std::vector<GTy> ScalarDyOprfGet_cache(std::shared_ptr<Context>& ctx,
                                       const ATy& scalar, size_t num);

std::vector<GTy> ScalarDyOprfSet(std::shared_ptr<Context>& ctx,
                                 const ATy& scalar, absl::Span<const PTy> in);
std::vector<GTy> ScalarDyOprfSet_cache(std::shared_ptr<Context>& ctx,
                                       const ATy& scalar,
                                       absl::Span<const PTy> in);

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
