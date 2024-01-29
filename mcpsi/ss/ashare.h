#pragma once
#include <vector>

#include "mcpsi/context/context.h"
#include "mcpsi/ss/public.h"
#include "mcpsi/ss/type.h"
#include "mcpsi/utils/field.h"
#include "mcpsi/utils/vec_op.h"

namespace mcpsi::internal {

// pure A-share operation

std::vector<ATy> AddAA(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const ATy> rhs);
std::vector<ATy> AddAA_cache(std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> lhs,
                             absl::Span<const ATy> rhs);

std::vector<ATy> SubAA(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const ATy> rhs);
std::vector<ATy> SubAA_cache(std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> lhs,
                             absl::Span<const ATy> rhs);

std::vector<ATy> MulAA(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const ATy> rhs);
std::vector<ATy> MulAA_cache(std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> lhs,
                             absl::Span<const ATy> rhs);

std::vector<ATy> DivAA(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const ATy> rhs);
std::vector<ATy> DivAA_cache(std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> lhs,
                             absl::Span<const ATy> rhs);

std::vector<ATy> NegA(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in);
std::vector<ATy> NegA_cache(std::shared_ptr<Context>& ctx,
                            absl::Span<const ATy> in);

std::vector<ATy> InvA(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in);
std::vector<ATy> InvA_cache(std::shared_ptr<Context>& ctx,
                            absl::Span<const ATy> in);

std::vector<ATy> ZerosA(std::shared_ptr<Context>& ctx, size_t num);
std::vector<ATy> ZerosA_cache(std::shared_ptr<Context>& ctx, size_t num);

std::vector<ATy> RandA(std::shared_ptr<Context>& ctx, size_t num);
std::vector<ATy> RandA_cache(std::shared_ptr<Context>& ctx, size_t num);

// A-share and Public Value operation

std::vector<ATy> AddAP(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const PTy> rhs);
std::vector<ATy> AddAP_cache(std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> lhs,
                             absl::Span<const PTy> rhs);

std::vector<ATy> SubAP(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const PTy> rhs);
std::vector<ATy> SubAP_cache(std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> lhs,
                             absl::Span<const PTy> rhs);

std::vector<ATy> MulAP(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const PTy> rhs);
std::vector<ATy> MulAP_cache(std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> lhs,
                             absl::Span<const PTy> rhs);

std::vector<ATy> DivAP(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const PTy> rhs);
std::vector<ATy> DivAP_cache(std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> lhs,
                             absl::Span<const PTy> rhs);

std::vector<ATy> AddPA(std::shared_ptr<Context>& ctx, absl::Span<const PTy> lhs,
                       absl::Span<const ATy> rhs);
std::vector<ATy> AddPA_cache(std::shared_ptr<Context>& ctx,
                             absl::Span<const PTy> lhs,
                             absl::Span<const ATy> rhs);

std::vector<ATy> SubPA(std::shared_ptr<Context>& ctx, absl::Span<const PTy> lhs,
                       absl::Span<const ATy> rhs);
std::vector<ATy> SubPA_cache(std::shared_ptr<Context>& ctx,
                             absl::Span<const PTy> lhs,
                             absl::Span<const ATy> rhs);

std::vector<ATy> MulPA(std::shared_ptr<Context>& ctx, absl::Span<const PTy> lhs,
                       absl::Span<const ATy> rhs);
std::vector<ATy> MulPA_cache(std::shared_ptr<Context>& ctx,
                             absl::Span<const PTy> lhs,
                             absl::Span<const ATy> rhs);

std::vector<ATy> DivPA(std::shared_ptr<Context>& ctx, absl::Span<const PTy> lhs,
                       absl::Span<const ATy> rhs);
std::vector<ATy> DivPA_cache(std::shared_ptr<Context>& ctx,
                             absl::Span<const PTy> lhs,
                             absl::Span<const ATy> rhs);

// A-share && Public Value Convert
std::vector<PTy> A2P(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in);
std::vector<PTy> A2P_cache(std::shared_ptr<Context>& ctx,
                           absl::Span<const ATy> in);

std::vector<ATy> P2A(std::shared_ptr<Context>& ctx, absl::Span<const PTy> in);
std::vector<ATy> P2A_cache(std::shared_ptr<Context>& ctx,
                           absl::Span<const PTy> in);

// special
std::vector<ATy> ShuffleAGet(std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> in);
std::vector<ATy> ShuffleAGet_cache(std::shared_ptr<Context>& ctx,
                                   absl::Span<const ATy> in);

std::vector<ATy> ShuffleASet(std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> in);
std::vector<ATy> ShuffleASet_cache(std::shared_ptr<Context>& ctx,
                                   absl::Span<const ATy> in);

std::vector<ATy> ShuffleA(std::shared_ptr<Context>& ctx,
                          absl::Span<const ATy> in);
std::vector<ATy> ShuffleA_cache(std::shared_ptr<Context>& ctx,
                                absl::Span<const ATy> in);

// shuffle inputs with same permutation
std::array<std::vector<ATy>, 2> ShuffleAGet(std::shared_ptr<Context>& ctx,
                                            absl::Span<const ATy> in0,
                                            absl::Span<const ATy> in1);
std::array<std::vector<ATy>, 2> ShuffleAGet_cache(std::shared_ptr<Context>& ctx,
                                                  absl::Span<const ATy> in0,
                                                  absl::Span<const ATy> in1);

std::array<std::vector<ATy>, 2> ShuffleASet(std::shared_ptr<Context>& ctx,
                                            absl::Span<const ATy> in0,
                                            absl::Span<const ATy> in1);
std::array<std::vector<ATy>, 2> ShuffleASet_cache(std::shared_ptr<Context>& ctx,
                                                  absl::Span<const ATy> in0,
                                                  absl::Span<const ATy> in1);

std::array<std::vector<ATy>, 2> ShuffleA(std::shared_ptr<Context>& ctx,
                                         absl::Span<const ATy> in0,
                                         absl::Span<const ATy> in1);
std::array<std::vector<ATy>, 2> ShuffleA_cache(std::shared_ptr<Context>& ctx,
                                               absl::Span<const ATy> in0,
                                               absl::Span<const ATy> in1);

// A-share Setter, return A-share ( in , in * key + r )
std::vector<ATy> SetA(std::shared_ptr<Context>& ctx, absl::Span<const PTy> in);
std::vector<ATy> SetA_cache(std::shared_ptr<Context>& ctx,
                            absl::Span<const PTy> in);
// A-share Getter, return A-share (  0 , in * key - r )
std::vector<ATy> GetA(std::shared_ptr<Context>& ctx, size_t num);
std::vector<ATy> GetA_cache(std::shared_ptr<Context>& ctx, size_t num);

std::vector<ATy> RandASet(std::shared_ptr<Context>& ctx, size_t num);
std::vector<ATy> RandASet_cache(std::shared_ptr<Context>& ctx, size_t num);

std::vector<ATy> RandAGet(std::shared_ptr<Context>& ctx, size_t num);
std::vector<ATy> RandAGet_cache(std::shared_ptr<Context>& ctx, size_t num);

std::vector<ATy> SumA(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in);
std::vector<ATy> SumA_cache(std::shared_ptr<Context>& ctx,
                            absl::Span<const ATy> in);

std::vector<ATy> FilterA(std::shared_ptr<Context>& ctx,
                         absl::Span<const ATy> in,
                         absl::Span<const size_t> indexes);
std::vector<ATy> FilterA_cache(std::shared_ptr<Context>& ctx,
                               absl::Span<const ATy> in,
                               absl::Span<const size_t> indexes);

std::vector<ATy> ZeroOneA(std::shared_ptr<Context>& ctx, size_t num);

std::vector<ATy> ZeroOneA_cache(std::shared_ptr<Context>& ctx, size_t num);

std::vector<ATy> ScalarMulPA(std::shared_ptr<Context>& ctx, const PTy& scalar,
                             absl::Span<const ATy> in);

std::vector<ATy> ScalarMulPA_cache(std::shared_ptr<Context>& ctx,
                                   const PTy& scalar, absl::Span<const ATy> in);

std::vector<ATy> ScalarMulAP(std::shared_ptr<Context>& ctx, const ATy& scalar,
                             absl::Span<const PTy> in);

std::vector<ATy> ScalarMulAP_cache(std::shared_ptr<Context>& ctx,
                                   const ATy& scalar, absl::Span<const PTy> in);

}  // namespace mcpsi::internal
