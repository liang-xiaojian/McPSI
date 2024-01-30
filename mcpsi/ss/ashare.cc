#include "mcpsi/ss/ashare.h"

#include <vector>

#include "mcpsi/cr/cr.h"
#include "mcpsi/cr/fake_cr.h"
#include "mcpsi/ss/protocol.h"
#include "mcpsi/utils/field.h"
#include "mcpsi/utils/vec_op.h"

namespace mcpsi::internal {

// --------------- AA ------------------

std::vector<ATy> AddAA([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const ATy> lhs, absl::Span<const ATy> rhs) {
  YACL_ENFORCE(lhs.size() == rhs.size());
  const size_t size = lhs.size();
  std::vector<ATy> ret(size);

  op::Add(
      absl::MakeConstSpan(reinterpret_cast<const PTy*>(lhs.data()), size * 2),
      absl::MakeConstSpan(reinterpret_cast<const PTy*>(rhs.data()), size * 2),
      absl::MakeSpan(reinterpret_cast<PTy*>(ret.data()), size * 2));

  return ret;
}

std::vector<ATy> AddAA_cache([[maybe_unused]] std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> lhs,
                             absl::Span<const ATy> rhs) {
  YACL_ENFORCE(lhs.size() == rhs.size());
  const size_t size = lhs.size();
  return std::vector<ATy>(size);
}

std::vector<ATy> SubAA([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const ATy> lhs, absl::Span<const ATy> rhs) {
  YACL_ENFORCE(lhs.size() == rhs.size());
  const size_t size = lhs.size();
  std::vector<ATy> ret(size);

  op::Sub(
      absl::MakeConstSpan(reinterpret_cast<const PTy*>(lhs.data()), size * 2),
      absl::MakeConstSpan(reinterpret_cast<const PTy*>(rhs.data()), size * 2),
      absl::MakeSpan(reinterpret_cast<PTy*>(ret.data()), size * 2));

  return ret;
}

std::vector<ATy> SubAA_cache([[maybe_unused]] std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> lhs,
                             absl::Span<const ATy> rhs) {
  YACL_ENFORCE(lhs.size() == rhs.size());
  const size_t size = lhs.size();
  return std::vector<ATy>(size);
}

std::vector<ATy> MulAA(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const ATy> rhs) {
  YACL_ENFORCE(lhs.size() == rhs.size());
  const size_t size = lhs.size();
  auto [a, b, c] = ctx->GetState<Correlation>()->BeaverTriple(size);
  auto u = SubAA(ctx, lhs, a);  // x-a
  auto v = SubAA(ctx, rhs, b);  // y-b
  auto u_p = A2P(ctx, u);
  auto v_p = A2P(ctx, v);

  // ret = c + x(y-b) + (x-a)y - (x-a)(y-b)
  auto xyb = MulAP(ctx, lhs, v_p);
  auto xay = MulPA(ctx, u_p, rhs);
  auto xayb = MulPP(ctx, u_p, v_p);
  auto oooo = P2A(ctx, absl::MakeSpan(xayb));
  c = AddAA(ctx, absl::MakeSpan(c), absl::MakeSpan(xyb));
  c = AddAA(ctx, absl::MakeSpan(c), absl::MakeSpan(xay));
  c = SubAA(ctx, absl::MakeSpan(c), absl::MakeSpan(oooo));
  return c;
}

std::vector<ATy> MulAA_cache([[maybe_unused]] std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> lhs,
                             absl::Span<const ATy> rhs) {
  YACL_ENFORCE(lhs.size() == rhs.size());
  const size_t size = lhs.size();
  ctx->GetState<Correlation>()->BeaverTriple_cache(size);
  std::vector<ATy> a(size);
  std::vector<ATy> b(size);
  std::vector<ATy> c(size);
  auto u = SubAA_cache(ctx, lhs, a);  // x-a
  auto v = SubAA_cache(ctx, rhs, b);  // y-b
  auto u_p = A2P_cache(ctx, u);
  auto v_p = A2P_cache(ctx, v);

  // ret = c + x(y-b) + (x-a)y - (x-a)(y-b)
  auto xyb = MulAP_cache(ctx, lhs, v_p);
  auto xay = MulPA_cache(ctx, u_p, rhs);
  auto xayb = MulPP_cache(ctx, u_p, v_p);
  auto oooo = P2A_cache(ctx, absl::MakeSpan(xayb));
  c = AddAA_cache(ctx, absl::MakeSpan(c), absl::MakeSpan(xyb));
  c = AddAA_cache(ctx, absl::MakeSpan(c), absl::MakeSpan(xay));
  c = SubAA_cache(ctx, absl::MakeSpan(c), absl::MakeSpan(oooo));
  return c;
}

std::vector<ATy> DivAA(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const ATy> rhs) {
  const size_t num = lhs.size();
  YACL_ENFORCE(num == rhs.size());
  auto inv = InvA(ctx, absl::MakeConstSpan(rhs));
  return MulAA(ctx, absl::MakeConstSpan(lhs), absl::MakeConstSpan(inv));
}

std::vector<ATy> DivAA_cache(std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> lhs,
                             absl::Span<const ATy> rhs) {
  const size_t num = lhs.size();
  YACL_ENFORCE(num == rhs.size());
  auto inv = InvA_cache(ctx, absl::MakeConstSpan(rhs));
  return MulAA_cache(ctx, absl::MakeConstSpan(lhs), absl::MakeConstSpan(inv));
}

std::vector<ATy> NegA([[maybe_unused]] std::shared_ptr<Context>& ctx,
                      absl::Span<const ATy> in) {
  const size_t size = in.size();
  std::vector<ATy> ret(size);
  op::Neg(
      absl::MakeConstSpan(reinterpret_cast<const PTy*>(in.data()), 2 * size),
      absl::MakeSpan(reinterpret_cast<PTy*>(ret.data()), 2 * size));
  return ret;
}

std::vector<ATy> NegA_cache([[maybe_unused]] std::shared_ptr<Context>& ctx,
                            absl::Span<const ATy> in) {
  const size_t size = in.size();
  return std::vector<ATy>(size);
}

std::vector<ATy> InvA(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in) {
  const size_t num = in.size();
  // r <-- random a-share
  auto r = RandA(ctx, num);
  // r * in
  auto mul = MulAA(ctx, absl::MakeConstSpan(r), absl::MakeConstSpan(in));
  // reveal r * in
  auto pub = A2P(ctx, absl::MakeConstSpan(mul));
  // inv = (r * in)^{-1}
  auto inv = InvP(ctx, absl::MakeConstSpan(pub));
  // r * inv = in^{-1}
  return MulAP(ctx, absl::MakeConstSpan(r), absl::MakeConstSpan(inv));
}

std::vector<ATy> InvA_cache(std::shared_ptr<Context>& ctx,
                            absl::Span<const ATy> in) {
  const size_t num = in.size();
  // r <-- random a-share
  auto r = RandA_cache(ctx, num);
  // r * in
  auto mul = MulAA_cache(ctx, absl::MakeConstSpan(r), absl::MakeConstSpan(in));
  // reveal r * in
  auto pub = A2P_cache(ctx, absl::MakeConstSpan(mul));
  // inv = (r * in)^{-1}
  auto inv = InvP_cache(ctx, absl::MakeConstSpan(pub));
  // r * inv = in^{-1}
  return MulAP_cache(ctx, absl::MakeConstSpan(r), absl::MakeConstSpan(inv));
}

std::vector<ATy> ZerosA(std::shared_ptr<Context>& ctx, size_t num) {
  std::vector<ATy> ret(num);

  auto prg_ptr = ctx->GetState<Prg>();
  op::Rand(*prg_ptr,
           absl::MakeSpan(reinterpret_cast<PTy*>(ret.data()), num * 2));
  if (ctx->GetRank() == 0) {
    op::Neg(
        absl::MakeConstSpan(reinterpret_cast<const PTy*>(ret.data()), num * 2),
        absl::MakeSpan(reinterpret_cast<PTy*>(ret.data()), num * 2));
  }
  return ret;
}

std::vector<ATy> ZerosA_cache([[maybe_unused]] std::shared_ptr<Context>& ctx,
                              size_t num) {
  return std::vector<ATy>(num);
}

std::vector<ATy> RandA(std::shared_ptr<Context>& ctx, size_t num) {
  return ctx->GetState<Correlation>()->RandomAuth(num).data;
}

std::vector<ATy> RandA_cache(std::shared_ptr<Context>& ctx, size_t num) {
  ctx->GetState<Correlation>()->RandomAuth_cache(num);
  return std::vector<ATy>(num);
}

// --------------- AP && PA ------------------

std::vector<ATy> AddAP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const ATy> lhs, absl::Span<const PTy> rhs) {
  const size_t size = lhs.size();
  YACL_ENFORCE(size == rhs.size());
  auto [val, mac] = Unpack(lhs);
  if (ctx->GetRank() == 0) {
    // val += rhs
    op::AddInplace(absl::MakeSpan(val), absl::MakeConstSpan(rhs));
  }
  std::vector<PTy> rhs_mac(size);
  op::ScalarMul(ctx->GetState<Protocol>()->GetKey(), absl::MakeConstSpan(rhs),
                absl::MakeSpan(rhs_mac));
  // mac += rhs_mac
  op::AddInplace(absl::MakeSpan(mac), absl::MakeConstSpan(rhs_mac));
  return Pack(absl::MakeSpan(val), absl::MakeSpan(mac));
}

std::vector<ATy> AddAP_cache([[maybe_unused]] std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> lhs,
                             absl::Span<const PTy> rhs) {
  const size_t size = lhs.size();
  YACL_ENFORCE(size == rhs.size());
  return std::vector<ATy>(size);
}

std::vector<ATy> SubAP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const ATy> lhs, absl::Span<const PTy> rhs) {
  const size_t size = lhs.size();
  YACL_ENFORCE(size == rhs.size());
  auto neg_rhs = NegP(ctx, rhs);
  return AddAP(ctx, lhs, neg_rhs);
}

std::vector<ATy> SubAP_cache([[maybe_unused]] std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> lhs,
                             absl::Span<const PTy> rhs) {
  const size_t size = lhs.size();
  YACL_ENFORCE(size == rhs.size());
  auto neg_rhs = NegP_cache(ctx, rhs);
  return AddAP_cache(ctx, lhs, neg_rhs);
}

std::vector<ATy> MulAP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const ATy> lhs, absl::Span<const PTy> rhs) {
  const size_t size = lhs.size();
  YACL_ENFORCE(size == rhs.size());
  auto [val, mac] = Unpack(absl::MakeConstSpan(lhs));
  op::MulInplace(absl::MakeSpan(val), absl::MakeConstSpan(rhs));
  op::MulInplace(absl::MakeSpan(mac), absl::MakeConstSpan(rhs));
  return Pack(absl::MakeSpan(val), absl::MakeSpan(mac));
}

std::vector<ATy> MulAP_cache([[maybe_unused]] std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> lhs,
                             absl::Span<const PTy> rhs) {
  const size_t size = lhs.size();
  YACL_ENFORCE(size == rhs.size());
  return std::vector<ATy>(size);
}

std::vector<ATy> DivAP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const ATy> lhs, absl::Span<const PTy> rhs) {
  const size_t size = lhs.size();
  YACL_ENFORCE(size == rhs.size());
  std::vector<PTy> inv(size);
  op::Inv(absl::MakeConstSpan(rhs), absl::MakeSpan(inv));
  return MulAP(ctx, lhs, inv);
}

std::vector<ATy> DivAP_cache([[maybe_unused]] std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> lhs,
                             absl::Span<const PTy> rhs) {
  const size_t size = lhs.size();
  YACL_ENFORCE(size == rhs.size());
  std::vector<PTy> inv(size);
  return MulAP_cache(ctx, lhs, inv);
}

std::vector<ATy> AddPA([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const PTy> lhs, absl::Span<const ATy> rhs) {
  return AddAP(ctx, rhs, lhs);
}

std::vector<ATy> AddPA_cache([[maybe_unused]] std::shared_ptr<Context>& ctx,
                             absl::Span<const PTy> lhs,
                             absl::Span<const ATy> rhs) {
  return AddAP_cache(ctx, rhs, lhs);
}

std::vector<ATy> SubPA([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const PTy> lhs, absl::Span<const ATy> rhs) {
  const size_t size = lhs.size();
  YACL_ENFORCE(size == rhs.size());
  auto neg_rhs = NegA(ctx, rhs);
  return AddPA(ctx, lhs, neg_rhs);
}

std::vector<ATy> SubPA_cache([[maybe_unused]] std::shared_ptr<Context>& ctx,
                             absl::Span<const PTy> lhs,
                             absl::Span<const ATy> rhs) {
  const size_t size = lhs.size();
  YACL_ENFORCE(size == rhs.size());
  auto neg_rhs = NegA_cache(ctx, rhs);
  return AddPA_cache(ctx, lhs, neg_rhs);
}

std::vector<ATy> MulPA([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const PTy> lhs, absl::Span<const ATy> rhs) {
  return MulAP(ctx, rhs, lhs);
}

std::vector<ATy> MulPA_cache([[maybe_unused]] std::shared_ptr<Context>& ctx,
                             absl::Span<const PTy> lhs,
                             absl::Span<const ATy> rhs) {
  return MulAP_cache(ctx, rhs, lhs);
}

std::vector<ATy> DivPA(std::shared_ptr<Context>& ctx, absl::Span<const PTy> lhs,
                       absl::Span<const ATy> rhs) {
  const size_t num = lhs.size();
  YACL_ENFORCE(num == rhs.size());
  auto inv = InvA(ctx, absl::MakeConstSpan(rhs));
  return MulPA(ctx, absl::MakeConstSpan(lhs), absl::MakeConstSpan(inv));
}

std::vector<ATy> DivPA_cache(std::shared_ptr<Context>& ctx,
                             absl::Span<const PTy> lhs,
                             absl::Span<const ATy> rhs) {
  const size_t num = lhs.size();
  YACL_ENFORCE(num == rhs.size());
  auto inv = InvA_cache(ctx, absl::MakeConstSpan(rhs));
  return MulPA_cache(ctx, absl::MakeConstSpan(lhs), absl::MakeConstSpan(inv));
}

// --------------- Conversion ------------------

std::vector<PTy> A2P(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in) {
  // TEST ME: whether is secure enough ???
  const size_t size = in.size();
  auto [val, mac] = Unpack(absl::MakeSpan(in));
  auto conn = ctx->GetState<Connection>();
  auto val_bv = yacl::ByteContainerView(val.data(), size * sizeof(PTy));
  std::vector<PTy> real_val(size);

  auto buf = conn->Exchange(val_bv);
  op::Add(absl::MakeConstSpan(reinterpret_cast<const PTy*>(buf.data()), size),
          absl::MakeConstSpan(val), absl::MakeSpan(real_val));

  // Generate Sync Seed After Open Value
  auto sync_seed = conn->SyncSeed();
  auto coef = op::Rand(sync_seed, size);
  // linear combination
  auto real_val_affine =
      op::InPro(absl::MakeSpan(coef), absl::MakeSpan(real_val));
  auto mac_affine = op::InPro(absl::MakeSpan(coef), absl::MakeSpan(mac));

  auto key = ctx->GetState<Protocol>()->GetKey();
  auto zero_mac = mac_affine - real_val_affine * key;

  auto remote_mac_int = conn->ExchangeWithCommit(zero_mac.GetVal());
  YACL_ENFORCE(zero_mac + PTy(remote_mac_int) == PTy::Zero());
  return real_val;
}

std::vector<PTy> A2P_cache(std::shared_ptr<Context>& ctx,
                           absl::Span<const ATy> in) {
  // TEST ME: whether is secure enough ???
  const size_t size = in.size();
  auto [val, mac] = Unpack(absl::MakeSpan(in));
  auto conn = ctx->GetState<Connection>();
  auto val_bv = yacl::ByteContainerView(val.data(), size * sizeof(PTy));
  std::vector<PTy> real_val(size);

  auto buf = conn->Exchange(val_bv);
  op::Add(absl::MakeConstSpan(reinterpret_cast<const PTy*>(buf.data()), size),
          absl::MakeConstSpan(val), absl::MakeSpan(real_val));

  // Generate Sync Seed After Open Value
  auto sync_seed = conn->SyncSeed();
  auto coef = op::Rand(sync_seed, size);
  // linear combination
  auto real_val_affine =
      op::InPro(absl::MakeSpan(coef), absl::MakeSpan(real_val));
  auto mac_affine = op::InPro(absl::MakeSpan(coef), absl::MakeSpan(mac));

  auto key = ctx->GetState<Protocol>()->GetKey();
  auto zero_mac = mac_affine - real_val_affine * key;

  auto remote_mac_int = conn->ExchangeWithCommit(zero_mac.GetVal());
  YACL_ENFORCE(zero_mac + PTy(remote_mac_int) == PTy::Zero());
  return real_val;
}

std::vector<ATy> P2A(std::shared_ptr<Context>& ctx, absl::Span<const PTy> in) {
  const size_t size = in.size();
  auto zero = ZerosA(ctx, size);
  auto [zero_val, zero_mac] = Unpack(zero);
  if (ctx->GetRank() == 0) {
    // zero_val += in
    op::AddInplace(absl::MakeSpan(zero_val), absl::MakeConstSpan(in));
  }
  auto in_mac = op::ScalarMul(ctx->GetState<Protocol>()->GetKey(), in);
  // zero_mac += in_mac
  op::AddInplace(absl::MakeSpan(zero_mac), absl::MakeConstSpan(in_mac));
  return Pack(absl::MakeSpan(zero_val), absl::MakeSpan(zero_mac));
}

std::vector<ATy> P2A_cache(std::shared_ptr<Context>& ctx,
                           absl::Span<const PTy> in) {
  const size_t size = in.size();
  auto zero = ZerosA(ctx, size);
  auto [zero_val, zero_mac] = Unpack(zero);
  if (ctx->GetRank() == 0) {
    // zero_val += in
    op::AddInplace(absl::MakeSpan(zero_val), absl::MakeConstSpan(in));
  }
  auto in_mac = op::ScalarMul(ctx->GetState<Protocol>()->GetKey(), in);
  // zero_mac += in_mac
  op::AddInplace(absl::MakeSpan(zero_mac), absl::MakeConstSpan(in_mac));
  return Pack(absl::MakeSpan(zero_val), absl::MakeSpan(zero_mac));
}

// --------------- Shuffle ------------------

std::vector<ATy> ShuffleAGet(std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> in) {
  const size_t num = in.size();
  // correlation
  // [Warning] low efficiency!!! optimize it
  auto [_a, _b] = ctx->GetState<Correlation>()->ShuffleGet(num, 2);
  auto val_a = absl::MakeSpan(_a).subspan(0, num);
  auto val_b = absl::MakeSpan(_b).subspan(0, num);
  auto mac_a = absl::MakeSpan(_a).subspan(num, num);
  auto mac_b = absl::MakeSpan(_b).subspan(num, num);

  auto [val_in, mac_in] = Unpack(absl::MakeConstSpan(in));

  op::AddInplace(absl::MakeSpan(val_a), absl::MakeConstSpan(val_in));
  op::AddInplace(absl::MakeSpan(mac_a), absl::MakeConstSpan(mac_in));

  auto conn = ctx->GetConnection();
  conn->SendAsync(
      ctx->NextRank(),
      yacl::ByteContainerView(val_a.data(), val_a.size() * sizeof(PTy)),
      "send:a+x val");
  conn->SendAsync(
      ctx->NextRank(),
      yacl::ByteContainerView(mac_a.data(), mac_a.size() * sizeof(PTy)),
      "send:a+x mac");

  return Pack(absl::MakeConstSpan(val_b), absl::MakeConstSpan(mac_b));
}

std::vector<ATy> ShuffleAGet_cache(std::shared_ptr<Context>& ctx,
                                   absl::Span<const ATy> in) {
  const size_t num = in.size();
  // correlation
  // [Warning] low efficiency!!! optimize it
  ctx->GetState<Correlation>()->ShuffleGet_cache(num, 2);
  return std::vector<ATy>(num);
}

std::vector<ATy> ShuffleASet(std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> in) {
  const size_t num = in.size();
  // correlation
  // [Warning] low efficiency!!! optimize it
  auto [_delta, perm] = ctx->GetState<Correlation>()->ShuffleSet(num, 2);
  auto val_delta = absl::MakeSpan(_delta).subspan(0, num);
  auto mac_delta = absl::MakeSpan(_delta).subspan(num, num);

  auto conn = ctx->GetConnection();
  auto val_buf = conn->Recv(ctx->NextRank(), "send:a");
  auto mac_buf = conn->Recv(ctx->NextRank(), "send:b");

  auto val_tmp = absl::MakeSpan(reinterpret_cast<PTy*>(val_buf.data()), num);
  auto mac_tmp = absl::MakeSpan(reinterpret_cast<PTy*>(mac_buf.data()), num);

  auto [val_in, mac_in] = Unpack(absl::MakeConstSpan(in));
  // val_tmp += val_in
  op::AddInplace(absl::MakeSpan(val_tmp), absl::MakeConstSpan(val_in));
  // mac_tmp += mac_in
  op::AddInplace(absl::MakeSpan(mac_tmp), absl::MakeConstSpan(mac_in));

  for (size_t i = 0; i < num; ++i) {
    val_delta[i] = val_delta[i] + val_tmp[perm[i]];
    mac_delta[i] = mac_delta[i] + mac_tmp[perm[i]];
  }
  return Pack(absl::MakeConstSpan(val_delta), absl::MakeConstSpan(mac_delta));
}

std::vector<ATy> ShuffleASet_cache(std::shared_ptr<Context>& ctx,
                                   absl::Span<const ATy> in) {
  const size_t num = in.size();
  // correlation
  // [Warning] low efficiency!!! optimize it
  ctx->GetState<Correlation>()->ShuffleSet_cache(num, 2);
  return std::vector<ATy>(num);
}

std::vector<ATy> ShuffleA(std::shared_ptr<Context>& ctx,
                          absl::Span<const ATy> in) {
  if (ctx->GetRank() == 0) {
    auto tmp = ShuffleASet(ctx, in);
    return ShuffleAGet(ctx, tmp);
  }
  auto tmp = ShuffleAGet(ctx, in);
  return ShuffleASet(ctx, tmp);
}

std::vector<ATy> ShuffleA_cache(std::shared_ptr<Context>& ctx,
                                absl::Span<const ATy> in) {
  if (ctx->GetRank() == 0) {
    auto tmp = ShuffleASet_cache(ctx, in);
    return ShuffleAGet_cache(ctx, tmp);
  }
  auto tmp = ShuffleAGet_cache(ctx, in);
  return ShuffleASet_cache(ctx, tmp);
}

// shuffle inputs with same permuation
std::array<std::vector<ATy>, 2> ShuffleAGet(std::shared_ptr<Context>& ctx,
                                            absl::Span<const ATy> in0,
                                            absl::Span<const ATy> in1) {
  const size_t num = in0.size();
  YACL_ENFORCE(in1.size() == num);
  // correlation
  // [Warning] low efficiency!!! optimize it
  auto [_a, _b] = ctx->GetState<Correlation>()->ShuffleGet(num, 4);
  auto val_a0 = absl::MakeSpan(_a).subspan(0 * num, num);
  auto val_b0 = absl::MakeSpan(_b).subspan(0 * num, num);
  auto mac_a0 = absl::MakeSpan(_a).subspan(1 * num, num);
  auto mac_b0 = absl::MakeSpan(_b).subspan(1 * num, num);

  auto val_a1 = absl::MakeSpan(_a).subspan(2 * num, num);
  auto val_b1 = absl::MakeSpan(_b).subspan(2 * num, num);
  auto mac_a1 = absl::MakeSpan(_a).subspan(3 * num, num);
  auto mac_b1 = absl::MakeSpan(_b).subspan(3 * num, num);

  auto [val_in0, mac_in0] = Unpack(absl::MakeConstSpan(in0));
  auto [val_in1, mac_in1] = Unpack(absl::MakeConstSpan(in1));

  op::AddInplace(absl::MakeSpan(val_a0), absl::MakeConstSpan(val_in0));
  op::AddInplace(absl::MakeSpan(mac_a0), absl::MakeConstSpan(mac_in0));
  op::AddInplace(absl::MakeSpan(val_a1), absl::MakeConstSpan(val_in1));
  op::AddInplace(absl::MakeSpan(mac_a1), absl::MakeConstSpan(mac_in1));

  auto conn = ctx->GetConnection();
  conn->SendAsync(
      ctx->NextRank(),
      yacl::ByteContainerView(val_a0.data(), val_a0.size() * sizeof(PTy)),
      "send:a+x val0");
  conn->SendAsync(
      ctx->NextRank(),
      yacl::ByteContainerView(mac_a0.data(), mac_a0.size() * sizeof(PTy)),
      "send:a+x mac0");
  conn->SendAsync(
      ctx->NextRank(),
      yacl::ByteContainerView(val_a1.data(), val_a1.size() * sizeof(PTy)),
      "send:a+x val1");
  conn->SendAsync(
      ctx->NextRank(),
      yacl::ByteContainerView(mac_a1.data(), mac_a1.size() * sizeof(PTy)),
      "send:a+x mac1");

  return {Pack(absl::MakeConstSpan(val_b0), absl::MakeConstSpan(mac_b0)),
          Pack(absl::MakeConstSpan(val_b1), absl::MakeConstSpan(mac_b1))};
}

std::array<std::vector<ATy>, 2> ShuffleAGet_cache(std::shared_ptr<Context>& ctx,
                                                  absl::Span<const ATy> in0,
                                                  absl::Span<const ATy> in1) {
  const size_t num = in0.size();
  YACL_ENFORCE(in1.size() == num);
  // correlation
  // [Warning] low efficiency!!! optimize it
  ctx->GetState<Correlation>()->ShuffleGet_cache(num, 4);
  return {std::vector<ATy>(num), std::vector<ATy>(num)};
}

std::array<std::vector<ATy>, 2> ShuffleASet(std::shared_ptr<Context>& ctx,
                                            absl::Span<const ATy> in0,
                                            absl::Span<const ATy> in1) {
  const size_t num = in0.size();
  YACL_ENFORCE(num == in1.size());
  // correlation
  // [Warning] low efficiency!!! optimize it
  auto [_delta, perm] = ctx->GetState<Correlation>()->ShuffleSet(num, 4);
  auto val_delta0 = absl::MakeSpan(_delta).subspan(0 * num, num);
  auto mac_delta0 = absl::MakeSpan(_delta).subspan(1 * num, num);
  auto val_delta1 = absl::MakeSpan(_delta).subspan(2 * num, num);
  auto mac_delta1 = absl::MakeSpan(_delta).subspan(3 * num, num);

  auto conn = ctx->GetConnection();
  auto val_buf0 = conn->Recv(ctx->NextRank(), "send:a0");
  auto mac_buf0 = conn->Recv(ctx->NextRank(), "send:b0");
  auto val_buf1 = conn->Recv(ctx->NextRank(), "send:a1");
  auto mac_buf1 = conn->Recv(ctx->NextRank(), "send:b1");

  auto val_tmp0 = absl::MakeSpan(reinterpret_cast<PTy*>(val_buf0.data()), num);
  auto mac_tmp0 = absl::MakeSpan(reinterpret_cast<PTy*>(mac_buf0.data()), num);
  auto val_tmp1 = absl::MakeSpan(reinterpret_cast<PTy*>(val_buf1.data()), num);
  auto mac_tmp1 = absl::MakeSpan(reinterpret_cast<PTy*>(mac_buf1.data()), num);

  auto [val_in0, mac_in0] = Unpack(absl::MakeConstSpan(in0));
  auto [val_in1, mac_in1] = Unpack(absl::MakeConstSpan(in1));

  op::AddInplace(absl::MakeSpan(val_tmp0), absl::MakeConstSpan(val_in0));
  op::AddInplace(absl::MakeSpan(mac_tmp0), absl::MakeConstSpan(mac_in0));
  op::AddInplace(absl::MakeSpan(val_tmp1), absl::MakeConstSpan(val_in1));
  op::AddInplace(absl::MakeSpan(mac_tmp1), absl::MakeConstSpan(mac_in1));

  for (size_t i = 0; i < num; ++i) {
    val_delta0[i] = val_delta0[i] + val_tmp0[perm[i]];
    mac_delta0[i] = mac_delta0[i] + mac_tmp0[perm[i]];
    val_delta1[i] = val_delta1[i] + val_tmp1[perm[i]];
    mac_delta1[i] = mac_delta1[i] + mac_tmp1[perm[i]];
  }
  return {
      Pack(absl::MakeConstSpan(val_delta0), absl::MakeConstSpan(mac_delta0)),
      Pack(absl::MakeConstSpan(val_delta1), absl::MakeConstSpan(mac_delta1))};
}

std::array<std::vector<ATy>, 2> ShuffleASet_cache(std::shared_ptr<Context>& ctx,
                                                  absl::Span<const ATy> in0,
                                                  absl::Span<const ATy> in1) {
  const size_t num = in0.size();
  YACL_ENFORCE(num == in1.size());
  // correlation
  // [Warning] low efficiency!!! optimize it
  ctx->GetState<Correlation>()->ShuffleSet_cache(num, 4);
  return {std::vector<ATy>(num), std::vector<ATy>(num)};
}

std::array<std::vector<ATy>, 2> ShuffleA(std::shared_ptr<Context>& ctx,
                                         absl::Span<const ATy> in0,
                                         absl::Span<const ATy> in1) {
  if (ctx->GetRank() == 0) {
    auto tmp = ShuffleASet(ctx, in0, in1);
    return ShuffleAGet(ctx, tmp[0], tmp[1]);
  }
  auto tmp = ShuffleAGet(ctx, in0, in1);
  return ShuffleASet(ctx, tmp[0], tmp[1]);
}

std::array<std::vector<ATy>, 2> ShuffleA_cache(std::shared_ptr<Context>& ctx,
                                               absl::Span<const ATy> in0,
                                               absl::Span<const ATy> in1) {
  if (ctx->GetRank() == 0) {
    auto tmp = ShuffleASet_cache(ctx, in0, in1);
    return ShuffleAGet_cache(ctx, tmp[0], tmp[1]);
  }
  auto tmp = ShuffleAGet_cache(ctx, in0, in1);
  return ShuffleASet_cache(ctx, tmp[0], tmp[1]);
}

// --------------- Special ------------------

// A-share Setter, return A-share ( in , in * key + r )
std::vector<ATy> SetA(std::shared_ptr<Context>& ctx, absl::Span<const PTy> in) {
  const size_t num = in.size();
  auto rand = RandASet(ctx, num);
  auto [val, mac] = Unpack(absl::MakeConstSpan(rand));
  // reuse, diff = in - val
  auto diff = op::Sub(absl::MakeConstSpan(in), absl::MakeConstSpan(val));
  ctx->GetConnection()->SendAsync(
      ctx->NextRank(), yacl::ByteContainerView(diff.data(), num * sizeof(PTy)),
      "SetA");
  // extra = diff * key
  auto diff_mac = op::ScalarMul(ctx->GetState<Protocol>()->GetKey(),
                                absl::MakeConstSpan(diff));
  // mac = diff_mac + mac
  op::AddInplace(absl::MakeSpan(mac), absl::MakeConstSpan(diff_mac));
  return Pack(absl::MakeConstSpan(in), absl::MakeConstSpan(mac));
}

std::vector<ATy> SetA_cache(std::shared_ptr<Context>& ctx,
                            absl::Span<const PTy> in) {
  const size_t num = in.size();
  return RandASet_cache(ctx, num);
}
// A-share Getter, return A-share (  0 , in * key - r )
std::vector<ATy> GetA(std::shared_ptr<Context>& ctx, size_t num) {
  auto zero = RandAGet(ctx, num);
  auto [val, mac] = Unpack(absl::MakeConstSpan(zero));
  auto buff = ctx->GetConnection()->Recv(ctx->NextRank(), "SetA");
  // diff
  auto diff = absl::MakeSpan(reinterpret_cast<PTy*>(buff.data()), num);
  auto diff_mac = op::ScalarMul(ctx->GetState<Protocol>()->GetKey(),
                                absl::MakeConstSpan(diff));
  // mac = diff_mac + mac
  op::AddInplace(absl::MakeSpan(mac), absl::MakeConstSpan(diff_mac));
  return Pack(absl::MakeConstSpan(val), absl::MakeConstSpan(mac));
}

std::vector<ATy> GetA_cache(std::shared_ptr<Context>& ctx, size_t num) {
  return RandAGet_cache(ctx, num);
}

std::vector<ATy> RandASet(std::shared_ptr<Context>& ctx, size_t num) {
  return ctx->GetState<Correlation>()->RandomSet(num).data;
}

std::vector<ATy> RandASet_cache(std::shared_ptr<Context>& ctx, size_t num) {
  ctx->GetState<Correlation>()->RandomSet_cache(num);
  return std::vector<ATy>(num);
}

std::vector<ATy> RandAGet(std::shared_ptr<Context>& ctx, size_t num) {
  return ctx->GetState<Correlation>()->RandomGet(num).data;
}

std::vector<ATy> RandAGet_cache(std::shared_ptr<Context>& ctx, size_t num) {
  ctx->GetState<Correlation>()->RandomGet_cache(num);
  return std::vector<ATy>(num);
}

std::vector<ATy> SumA([[maybe_unused]] std::shared_ptr<Context>& ctx,
                      absl::Span<const ATy> in) {
  const size_t num = in.size();
  std::vector<ATy> ret(1, {0, 0});
  for (size_t i = 0; i < num; ++i) {
    ret[0].val = ret[0].val + in[i].val;
    ret[0].mac = ret[0].mac + in[i].mac;
  }
  return ret;
}

std::vector<ATy> SumA_cache([[maybe_unused]] std::shared_ptr<Context>& ctx,
                            absl::Span<const ATy> in) {
  const size_t num = in.size();
  return std::vector<ATy>(num);
}

std::vector<ATy> FilterA([[maybe_unused]] std::shared_ptr<Context>& ctx,
                         absl::Span<const ATy> in,
                         absl::Span<const size_t> indexes) {
  const size_t ret_num = indexes.size();
  const size_t in_num = in.size();
  YACL_ENFORCE(ret_num <= in_num);
  std::vector<ATy> ret(ret_num);
  for (size_t i = 0; i < ret_num; ++i) {
    ret[i] = in[indexes[i]];
  }
  return ret;
}

std::vector<ATy> FilterA_cache([[maybe_unused]] std::shared_ptr<Context>& ctx,
                               [[maybe_unused]] absl::Span<const ATy> in,
                               absl::Span<const size_t> indexes) {
  const size_t ret_num = indexes.size();
  return std::vector<ATy>(ret_num);
}

// one or zero
std::vector<ATy> ZeroOneA(std::shared_ptr<Context>& ctx, size_t num) {
  auto r = RandA(ctx, num);
  // s = r * r
  auto s = MulAA(ctx, r, r);
  // reveal s
  auto p = A2P(ctx, s);
  auto root = op::Sqrt(absl::MakeSpan(p));

  auto tmp = DivAP(ctx, r, root);
  auto one = OnesP(ctx, num);
  auto res = AddAP(ctx, tmp, one);

  auto inv_two = PTy::Inv(PTy(2));
  return ScalarMulPA(ctx, inv_two, res);
}

std::vector<ATy> ZeroOneA_cache(std::shared_ptr<Context>& ctx, size_t num) {
  auto r = RandA_cache(ctx, num);
  // s = r * r
  auto s = MulAA_cache(ctx, r, r);
  // reveal s
  auto p = A2P_cache(ctx, s);
  auto root = op::Sqrt(absl::MakeSpan(p));

  auto tmp = DivAP_cache(ctx, r, root);
  auto one = OnesP_cache(ctx, num);
  auto res = AddAP_cache(ctx, tmp, one);

  auto inv_two = PTy::Inv(PTy(2));
  return ScalarMulPA_cache(ctx, inv_two, res);
}

std::vector<ATy> ScalarMulPA([[maybe_unused]] std::shared_ptr<Context>& ctx,
                             const PTy& scalar, absl::Span<const ATy> in) {
  const size_t num = in.size();
  std::vector<ATy> ret(in.begin(), in.end());
  op::ScalarMulInplace(
      scalar, absl::MakeSpan(reinterpret_cast<PTy*>(ret.data()), 2 * num));
  return ret;
}

std::vector<ATy> ScalarMulPA_cache(
    [[maybe_unused]] std::shared_ptr<Context>& ctx,
    [[maybe_unused]] const PTy& scalar, absl::Span<const ATy> in) {
  const size_t num = in.size();
  return std::vector<ATy>(num);
}

std::vector<ATy> ScalarMulAP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                             const ATy& scalar, absl::Span<const PTy> in) {
  auto val = op::ScalarMul(scalar.val, in);
  auto mac = op::ScalarMul(scalar.mac, in);
  return Pack(val, mac);
}

std::vector<ATy> ScalarMulAP_cache(
    [[maybe_unused]] std::shared_ptr<Context>& ctx,
    [[maybe_unused]] const ATy& scalar, absl::Span<const PTy> in) {
  const size_t num = in.size();
  return std::vector<ATy>(num);
}

std::pair<std::vector<ATy>, std::vector<ATy>> RandFairA(
    std::shared_ptr<Context>& ctx, size_t num) {
  typedef decltype(std::declval<internal::PTy>().GetVal()) INTEGER;

  const size_t bits_num = num * sizeof(INTEGER) * 8;
  auto bits = ZeroOneA(ctx, bits_num);
  auto bits_span = absl::MakeSpan(bits);
  std::vector<ATy> ret(num, {PTy::Zero(), PTy::Zero()});

  auto scalar = PTy::One();
  for (size_t i = 0; i < sizeof(INTEGER) * 8; ++i) {
    auto cur_bits = bits_span.subspan(i * num, num);
    auto tmp = ScalarMulPA(ctx, scalar, cur_bits);
    op::AddInplace(
        absl::MakeSpan(reinterpret_cast<PTy*>(ret.data()), 2 * num),
        absl::MakeConstSpan(reinterpret_cast<const PTy*>(tmp.data()), 2 * num));
    scalar = scalar * PTy(2);
  }
  return {ret, bits};
}

std::pair<std::vector<ATy>, std::vector<ATy>> RandFairA_cache(
    std::shared_ptr<Context>& ctx, size_t num) {
  typedef decltype(std::declval<internal::PTy>().GetVal()) INTEGER;

  const size_t bits_num = num * sizeof(INTEGER) * 8;
  auto bits = ZeroOneA_cache(ctx, bits_num);
  auto bits_span = absl::MakeSpan(bits);
  std::vector<ATy> ret(num, {PTy::Zero(), PTy::Zero()});

  auto scalar = PTy::One();
  for (size_t i = 0; i < sizeof(INTEGER) * 8; ++i) {
    auto cur_bits = bits_span.subspan(i * num, num);
    [[maybe_unused]] auto tmp = ScalarMulPA_cache(ctx, scalar, cur_bits);
  }
  return {ret, bits};
}

std::vector<PTy> FairA2P(std::shared_ptr<Context>& ctx,
                         absl::Span<const ATy> in, absl::Span<const ATy> bits) {
  typedef decltype(std::declval<internal::PTy>().GetVal()) INTEGER;
  const size_t num = in.size();
  YACL_ENFORCE(num * sizeof(INTEGER) * 8 == bits.size());

  std::vector<PTy> ret(num, PTy::Zero());
  auto scalar = PTy::One();
  for (size_t i = 0; i < sizeof(INTEGER) * 8; ++i) {
    auto bits_p = A2P(ctx, bits.subspan(num * i, num));
    for (const auto& bit_p : bits_p) {
      YACL_ENFORCE(bit_p == PTy::One() || bit_p == PTy::Zero());
    }
    auto tmp = op::ScalarMul(scalar, bits_p);
    scalar = scalar * PTy(2);
    op::AddInplace(absl::MakeSpan(ret), absl::MakeConstSpan(tmp));
  }

  auto check = SubAP(ctx, in, ret);
  auto zeros = A2P(ctx, check);
  for (const auto& zero : zeros) {
    YACL_ENFORCE(zero == PTy::Zero());
  }

  return ret;
}

std::vector<PTy> FairA2P_cache(std::shared_ptr<Context>& ctx,
                               absl::Span<const ATy> in,
                               absl::Span<const ATy> bits) {
  typedef decltype(std::declval<internal::PTy>().GetVal()) INTEGER;
  const size_t num = in.size();
  YACL_ENFORCE(num * sizeof(INTEGER) * 8 == bits.size());

  std::vector<PTy> ret(num, PTy::Zero());
  for (size_t i = 0; i < sizeof(INTEGER) * 8; ++i) {
    [[maybe_unused]] auto bits_p = A2P_cache(ctx, bits.subspan(num * i, num));
  }

  auto check = SubAP_cache(ctx, in, ret);
  [[maybe_unused]] auto zeros = A2P_cache(ctx, check);
  return ret;
}

}  // namespace mcpsi::internal
