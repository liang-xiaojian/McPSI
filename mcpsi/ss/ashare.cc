#include "mcpsi/ss/ashare.h"

#include <vector>

#include "mcpsi/cr/fake_cr.h"
#include "mcpsi/ss/protocol.h"
#include "mcpsi/utils/field.h"
#include "mcpsi/utils/vec_op.h"

namespace mcpsi::internal {
std::vector<ATy> AddAA([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const ATy> lhs, absl::Span<const ATy> rhs) {
  YACL_ENFORCE(lhs.size() == rhs.size());
  const size_t size = lhs.size();
  std::vector<ATy> ret(size);

  Add(absl::MakeConstSpan(reinterpret_cast<const PTy*>(lhs.data()), size * 2),
      absl::MakeConstSpan(reinterpret_cast<const PTy*>(rhs.data()), size * 2),
      absl::MakeSpan(reinterpret_cast<PTy*>(ret.data()), size * 2));

  return ret;
}

std::vector<ATy> SubAA([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const ATy> lhs, absl::Span<const ATy> rhs) {
  YACL_ENFORCE(lhs.size() == rhs.size());
  const size_t size = lhs.size();
  std::vector<ATy> ret(size);

  Sub(absl::MakeConstSpan(reinterpret_cast<const PTy*>(lhs.data()), size * 2),
      absl::MakeConstSpan(reinterpret_cast<const PTy*>(rhs.data()), size * 2),
      absl::MakeSpan(reinterpret_cast<PTy*>(ret.data()), size * 2));

  return ret;
}

std::vector<ATy> MulAA(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const ATy> rhs) {
  YACL_ENFORCE(lhs.size() == rhs.size());
  const size_t size = lhs.size();
  auto [a, b, c] = ctx->GetState<FakeCorrelation>()->BeaverTriple(size);
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

std::vector<ATy> DivAA(std::shared_ptr<Context>& ctx, absl::Span<const ATy> lhs,
                       absl::Span<const ATy> rhs) {
  const size_t num = lhs.size();
  YACL_ENFORCE(num == rhs.size());
  auto inv = InvA(ctx, absl::MakeConstSpan(rhs));
  return MulAA(ctx, absl::MakeConstSpan(lhs), absl::MakeConstSpan(inv));
}

std::vector<ATy> NegA([[maybe_unused]] std::shared_ptr<Context>& ctx,
                      absl::Span<const ATy> in) {
  const size_t size = in.size();
  std::vector<ATy> ret(size);
  Neg(absl::MakeConstSpan(reinterpret_cast<const PTy*>(in.data()), 2 * size),
      absl::MakeSpan(reinterpret_cast<PTy*>(ret.data()), 2 * size));
  return ret;
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

std::vector<ATy> ZerosA(std::shared_ptr<Context>& ctx, size_t num) {
  std::vector<ATy> ret(num);

  auto prg_ptr = ctx->GetState<Prg>();
  Rand(*prg_ptr, absl::MakeSpan(reinterpret_cast<PTy*>(ret.data()), num * 2));
  if (ctx->GetRank() == 0) {
    Neg(absl::MakeConstSpan(reinterpret_cast<const PTy*>(ret.data()), num * 2),
        absl::MakeSpan(reinterpret_cast<PTy*>(ret.data()), num * 2));
  }
  return ret;
}

std::vector<ATy> RandA(std::shared_ptr<Context>& ctx, size_t num) {
  return ctx->GetState<FakeCorrelation>()->RandomAuth(num);
}

std::vector<ATy> AddAP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const ATy> lhs, absl::Span<const PTy> rhs) {
  const size_t size = lhs.size();
  YACL_ENFORCE(size == rhs.size());
  auto [val, mac] = Unpack(lhs);
  if (ctx->GetRank() == 0) {
    Add(absl::MakeConstSpan(val), absl::MakeConstSpan(rhs),
        absl::MakeSpan(val));
  }
  std::vector<PTy> rhs_mac(size);
  ScalarMul(ctx->GetState<Protocol>()->GetKey(), absl::MakeConstSpan(rhs),
            absl::MakeSpan(rhs_mac));
  Add(absl::MakeConstSpan(mac), absl::MakeConstSpan(rhs_mac),
      absl::MakeSpan(mac));
  return Pack(absl::MakeSpan(val), absl::MakeSpan(mac));
}

std::vector<ATy> SubAP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const ATy> lhs, absl::Span<const PTy> rhs) {
  const size_t size = lhs.size();
  YACL_ENFORCE(size == rhs.size());
  auto neg_rhs = NegP(ctx, rhs);
  return AddAP(ctx, lhs, neg_rhs);
}

std::vector<ATy> MulAP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const ATy> lhs, absl::Span<const PTy> rhs) {
  const size_t size = lhs.size();
  YACL_ENFORCE(size == rhs.size());
  auto [val, mac] = Unpack(absl::MakeConstSpan(lhs));
  Mul(absl::MakeConstSpan(rhs), absl::MakeConstSpan(val), absl::MakeSpan(val));
  Mul(absl::MakeConstSpan(rhs), absl::MakeConstSpan(mac), absl::MakeSpan(mac));
  return Pack(absl::MakeSpan(val), absl::MakeSpan(mac));
}

std::vector<ATy> DivAP([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const ATy> lhs, absl::Span<const PTy> rhs) {
  const size_t size = lhs.size();
  YACL_ENFORCE(size == rhs.size());
  std::vector<PTy> inv(size);
  Inv(absl::MakeConstSpan(rhs), absl::MakeSpan(inv));
  return MulAP(ctx, lhs, inv);
}

std::vector<ATy> AddPA([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const PTy> lhs, absl::Span<const ATy> rhs) {
  return AddAP(ctx, rhs, lhs);
}

std::vector<ATy> SubPA([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const PTy> lhs, absl::Span<const ATy> rhs) {
  const size_t size = lhs.size();
  YACL_ENFORCE(size == rhs.size());
  auto neg_rhs = NegA(ctx, rhs);
  return AddPA(ctx, lhs, neg_rhs);
}

std::vector<ATy> MulPA([[maybe_unused]] std::shared_ptr<Context>& ctx,
                       absl::Span<const PTy> lhs, absl::Span<const ATy> rhs) {
  return MulAP(ctx, rhs, lhs);
}

std::vector<ATy> DivPA(std::shared_ptr<Context>& ctx, absl::Span<const PTy> lhs,
                       absl::Span<const ATy> rhs) {
  const size_t num = lhs.size();
  YACL_ENFORCE(num == rhs.size());
  auto inv = InvA(ctx, absl::MakeConstSpan(rhs));
  return MulPA(ctx, absl::MakeConstSpan(lhs), absl::MakeConstSpan(inv));
}

std::vector<PTy> A2P(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in) {
  // TEST ME: whether is secure enough ???
  const size_t size = in.size();
  auto [val, mac] = Unpack(absl::MakeSpan(in));
  auto lctx = ctx->GetState<Connection>();
  auto val_bv = yacl::ByteContainerView(val.data(), size * sizeof(PTy));
  std::vector<PTy> real_val(size);
  if (ctx->GetRank() == 0) {
    lctx->SendAsync(ctx->NextRank(), val_bv, "A2P:0");
    auto buf = lctx->Recv(ctx->NextRank(), "A2P:1");
    Add(absl::MakeConstSpan(reinterpret_cast<const PTy*>(buf.data()), size),
        absl::MakeConstSpan(val), absl::MakeSpan(real_val));
  } else {
    auto buf = lctx->Recv(ctx->NextRank(), "A2P:0");
    lctx->SendAsync(ctx->NextRank(), val_bv, "A2P:1");
    Add(absl::MakeConstSpan(reinterpret_cast<const PTy*>(buf.data()), size),
        absl::MakeConstSpan(val), absl::MakeSpan(real_val));
  }
  // Generate Sync Seed After Open Value
  auto sync_seed = lctx->SyncSeed();
  auto coef = Rand(sync_seed, size);
  // linear combination
  auto real_val_affine = InPro(absl::MakeSpan(coef), absl::MakeSpan(real_val));
  auto mac_affine = InPro(absl::MakeSpan(coef), absl::MakeSpan(mac));

  auto key = ctx->GetState<Protocol>()->GetKey();
  auto zero_mac = mac_affine - real_val_affine * key;

  auto remote_mac_u64 = lctx->ExchangeWithCommit(zero_mac.GetVal());
  YACL_ENFORCE(zero_mac + kFp64(remote_mac_u64) == kFp64::Zero());
  return real_val;
}

std::vector<ATy> P2A(std::shared_ptr<Context>& ctx, absl::Span<const PTy> in) {
  const size_t size = in.size();
  auto zero = ZerosA(ctx, size);
  auto [zero_val, zero_mac] = Unpack(zero);
  if (ctx->GetRank() == 0) {
    Add(absl::MakeConstSpan(zero_val), absl::MakeConstSpan(in),
        absl::MakeSpan(zero_val));
  }
  auto in_mac = ScalarMul(ctx->GetState<Protocol>()->GetKey(), in);
  Add(absl::MakeConstSpan(zero_mac), absl::MakeConstSpan(in_mac),
      absl::MakeSpan(zero_mac));
  return Pack(absl::MakeSpan(zero_val), absl::MakeSpan(zero_mac));
}

std::vector<ATy> ShuffleAGet(std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> in) {
  const size_t num = in.size();
  // correlation
  auto [val_a, val_b] = ctx->GetState<FakeCorrelation>()->ShuffleGet(num);
  auto [mac_a, mac_b] = ctx->GetState<FakeCorrelation>()->ShuffleGet(num);

  auto [val_in, mac_in] = Unpack(absl::MakeConstSpan(in));

  Add(absl::MakeConstSpan(val_a), absl::MakeConstSpan(val_in),
      absl::MakeSpan(val_a));
  Add(absl::MakeConstSpan(mac_a), absl::MakeConstSpan(mac_in),
      absl::MakeSpan(mac_a));

  auto lctx = ctx->GetConnection();
  lctx->SendAsync(
      ctx->NextRank(),
      yacl::ByteContainerView(val_a.data(), val_a.size() * sizeof(PTy)),
      "send:a+x val");
  lctx->SendAsync(
      ctx->NextRank(),
      yacl::ByteContainerView(mac_a.data(), mac_a.size() * sizeof(PTy)),
      "send:a+x mac");

  return Pack(absl::MakeConstSpan(val_b), absl::MakeConstSpan(mac_b));
}

std::vector<ATy> ShuffleASet(std::shared_ptr<Context>& ctx,
                             absl::Span<const ATy> in,
                             absl::Span<const size_t> perm) {
  const size_t num = in.size();
  YACL_ENFORCE(num == perm.size());
  // correlation
  auto val_delta = ctx->GetState<FakeCorrelation>()->ShuffleSet(perm);
  auto mac_delta = ctx->GetState<FakeCorrelation>()->ShuffleSet(perm);

  auto lctx = ctx->GetConnection();
  auto val_buf = lctx->Recv(ctx->NextRank(), "send:a");
  auto mac_buf = lctx->Recv(ctx->NextRank(), "send:b");

  auto val_tmp = absl::MakeSpan(reinterpret_cast<PTy*>(val_buf.data()), num);
  auto mac_tmp = absl::MakeSpan(reinterpret_cast<PTy*>(mac_buf.data()), num);

  auto [val_in, mac_in] = Unpack(absl::MakeConstSpan(in));
  Add(absl::MakeConstSpan(val_in), absl::MakeConstSpan(val_tmp),
      absl::MakeSpan(val_tmp));
  Add(absl::MakeConstSpan(mac_in), absl::MakeConstSpan(mac_tmp),
      absl::MakeSpan(mac_tmp));

  for (size_t i = 0; i < num; ++i) {
    val_delta[i] = val_delta[i] + val_tmp[perm[i]];
    mac_delta[i] = mac_delta[i] + mac_tmp[perm[i]];
  }
  return Pack(absl::MakeConstSpan(val_delta), absl::MakeConstSpan(mac_delta));
}

std::vector<ATy> ShuffleA(std::shared_ptr<Context>& ctx,
                          absl::Span<const ATy> in,
                          absl::Span<const size_t> perm) {
  if (ctx->GetRank() == 0) {
    auto tmp = ShuffleASet(ctx, in, perm);
    return ShuffleAGet(ctx, tmp);
  }
  auto tmp = ShuffleAGet(ctx, in);
  return ShuffleASet(ctx, tmp, perm);
}

// A-share Setter, return A-share ( in , in * key + r )
std::vector<ATy> SetA(std::shared_ptr<Context>& ctx, absl::Span<const PTy> in) {
  const size_t num = in.size();
  auto rand = RandASet(ctx, num);
  auto [val, mac] = Unpack(absl::MakeConstSpan(rand));
  // reuse, diff = in - val
  auto diff = Sub(absl::MakeConstSpan(in), absl::MakeConstSpan(val));
  ctx->GetConnection()->SendAsync(
      ctx->NextRank(), yacl::ByteContainerView(diff.data(), num * sizeof(PTy)),
      "SetA");
  // extra = diff * key
  auto diff_mac =
      ScalarMul(ctx->GetState<Protocol>()->GetKey(), absl::MakeConstSpan(diff));
  // mac = diff_mac + mac
  Add(absl::MakeConstSpan(mac), absl::MakeConstSpan(diff_mac),
      absl::MakeSpan(mac));
  return Pack(absl::MakeConstSpan(in), absl::MakeConstSpan(mac));
}
// A-share Getter, return A-share (  0 , in * key - r )
std::vector<ATy> GetA(std::shared_ptr<Context>& ctx, size_t num) {
  auto zero = RandAGet(ctx, num);
  auto [val, mac] = Unpack(absl::MakeConstSpan(zero));
  auto buff = ctx->GetConnection()->Recv(ctx->NextRank(), "SetA");
  // diff
  auto diff = absl::MakeSpan(reinterpret_cast<PTy*>(buff.data()), num);
  auto diff_mac =
      ScalarMul(ctx->GetState<Protocol>()->GetKey(), absl::MakeConstSpan(diff));
  // mac = diff_mac + mac
  Add(absl::MakeConstSpan(mac), absl::MakeConstSpan(diff_mac),
      absl::MakeSpan(mac));
  return Pack(absl::MakeConstSpan(val), absl::MakeConstSpan(mac));
}

std::vector<ATy> RandASet(std::shared_ptr<Context>& ctx, size_t num) {
  return ctx->GetState<FakeCorrelation>()->RandomSet(num);
}

std::vector<ATy> RandAGet(std::shared_ptr<Context>& ctx, size_t num) {
  return ctx->GetState<FakeCorrelation>()->RandomGet(num);
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

}  // namespace mcpsi::internal
