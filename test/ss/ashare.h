#pragma once
#include <vector>

#include "test/context/context.h"
#include "test/ss/public.h"
#include "test/ss/type.h"
#include "test/utils/field.h"
#include "test/utils/vec_op.h"

namespace test::internal {

// pure A-share operation

std::vector<ATy> AddAA(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const ATy> rhs);

std::vector<ATy> SubAA(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const ATy> rhs);

std::vector<ATy> MulAA(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const ATy> rhs);

std::vector<ATy> DivAA(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const ATy> rhs);

std::vector<ATy> NegA(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in);

std::vector<ATy> InvA(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in);

std::vector<ATy> ZerosA(std::shared_ptr<Context>& ctx, size_t num);

std::vector<ATy> RandA(std::shared_ptr<Context>& ctx, size_t num);

// A-share and Public Value operation

std::vector<ATy> AddAP(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const PTy> rhs);

std::vector<ATy> SubAP(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const PTy> rhs);

std::vector<ATy> MulAP(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const PTy> rhs);

std::vector<ATy> DivAP(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const PTy> rhs);

std::vector<ATy> AddPA(std::shared_ptr<Context>& ctx, absl::Span<const PTy> lhs,
                       absl::Span<const ATy> rhs);

std::vector<ATy> SubPA(std::shared_ptr<Context>& ctx, absl::Span<const PTy> lhs,
                       absl::Span<const ATy> rhs);

std::vector<ATy> MulPA(std::shared_ptr<Context>& ctx, absl::Span<const PTy> lhs,
                       absl::Span<const ATy> rhs);

std::vector<ATy> DivPA(std::shared_ptr<Context>& ctx, absl::Span<const PTy> lhs,
                       absl::Span<const ATy> rhs);

// A-share && Public Value Convert
std::vector<PTy> A2P(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in);

std::vector<ATy> P2A(std::shared_ptr<Context>& ctx, absl::Span<const PTy> in);

// special
std::vector<ATy> ShuffleAGet(std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> in);

std::vector<ATy> ShuffleASet(std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> in,
                             absl::Span<const size_t> perm);

std::vector<ATy> ShuffleA(std::shared_ptr<Context>& ctx,
                          absl::Span<const ATy> in,
                          absl::Span<const size_t> perm);
// truly shuffle
std::vector<ATy> inline ShuffleA(std::shared_ptr<Context>& ctx,
                                 absl::Span<const ATy> in) {
  auto perm = GenPerm(in.size());
  return ShuffleA(ctx, in, absl::MakeSpan(perm));
}

// A-share Setter, return A-share ( in , in * key + r )
std::vector<ATy> SetA(std::shared_ptr<Context>& ctx, absl::Span<const PTy> in);
// A-share Getter, return A-share (  0 , in * key - r )
std::vector<ATy> GetA(std::shared_ptr<Context>& ctx, size_t num);

std::vector<ATy> RandASet(std::shared_ptr<Context>& ctx, size_t num);

std::vector<ATy> RandAGet(std::shared_ptr<Context>& ctx, size_t num);

}  // namespace test::internal
