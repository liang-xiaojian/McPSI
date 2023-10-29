#include "mcpsi/ss/gshare.h"

#include "mcpsi/ss/protocol.h"
#include "yacl/base/byte_container_view.h"
#include "yacl/math/mpint/mp_int.h"

namespace mcpsi::internal {

std::vector<MTy> A2M(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in) {
  const size_t num = in.size();

  auto prot = ctx->GetState<Protocol>();
  auto prf_mod = prot->GetPrfMod();  // module for PRF
  auto prf_g = prot->GetPrfG();      // generator for PRF
  auto prf_k = prot->GetPrfK();      // distributed key for PRF (A-share)
  auto ext_k = std::vector<ATy>(num, prf_k);
  // in + k
  auto add = AddAA(ctx, in, ext_k);
  // 1 / (in + k)
  auto inv = InvA(ctx, add);

  auto ret = std::vector<MTy>(num);
  // Unpack by hand
  for (size_t i = 0; i < num; ++i) {
    ret[i].val = prf_g.PowMod(ym::MPInt(inv[i].val.GetVal()), prf_mod);
    ret[i].mac = prf_g.PowMod(ym::MPInt(inv[i].mac.GetVal()), prf_mod);
  }
  return ret;
}

// TEST ME: whether is secure enough ???
std::vector<GTy> M2G(std::shared_ptr<Context>& ctx, absl::Span<const MTy> in) {
  const size_t num = in.size();
  auto spdz_key = ctx->GetState<Protocol>()->GetKey();
  auto prf_mod = ctx->GetState<Protocol>()->GetPrfMod();

  std::vector<GTy> ret(num);
  // Convert MTy (val) to uint64_t by hand
  std::vector<uint64_t> send_buff(num);
  for (size_t i = 0; i < num; ++i) {
    send_buff[i] = in[i].val.Get<uint64_t>();
  }
  auto val_bv =
      yacl::ByteContainerView(send_buff.data(), num * sizeof(uint64_t));

  auto lctx = ctx->GetConnection();
  if (ctx->GetRank() == 0) {
    lctx->SendAsync(ctx->NextRank(), val_bv, "M2G:0");
    auto buf = lctx->Recv(ctx->NextRank(), "M2G:1");
    auto span = absl::MakeSpan(reinterpret_cast<uint64_t*>(buf.data()), num);
    for (size_t i = 0; i < num; ++i) {
      ret[i] = in[i].val.MulMod(ym::MPInt(span[i]), prf_mod);
    }
  } else {
    auto buf = lctx->Recv(ctx->NextRank(), "M2G:0");
    lctx->SendAsync(ctx->NextRank(), val_bv, "M2G:1");
    auto span = absl::MakeSpan(reinterpret_cast<uint64_t*>(buf.data()), num);
    for (size_t i = 0; i < num; ++i) {
      ret[i] = in[i].val.MulMod(ym::MPInt(span[i]), prf_mod);
    }
  }
  auto sync_seed = lctx->SyncSeed();
  auto coef = Rand(sync_seed, num);
  auto u64_coef = absl::MakeSpan(reinterpret_cast<uint64_t*>(coef.data()), num);
  GTy real_val_affine{1};
  GTy mac_affine{1};
  // compute the linear combination by hand
  for (size_t i = 0; i < num; ++i) {
    real_val_affine = real_val_affine.MulMod(
        ret[i].PowMod(ym::MPInt(u64_coef[i]), prf_mod), prf_mod);
    mac_affine = mac_affine.MulMod(
        in[i].mac.PowMod(ym::MPInt(u64_coef[i]), prf_mod), prf_mod);
  }

  auto local_mac_GTy =
      real_val_affine.PowMod(ym::MPInt(spdz_key.GetVal()), prf_mod);

  auto zero_mac_GTy =
      mac_affine.MulMod(local_mac_GTy.InvertMod(prf_mod), prf_mod);
  auto zero_mac_u64 = zero_mac_GTy.Get<uint64_t>();
  auto remote_mac_u64 = lctx->ExchangeWithCommit(zero_mac_u64);

  YACL_ENFORCE(zero_mac_GTy.MulMod(ym::MPInt(remote_mac_u64), prf_mod) ==
               ym::MPInt(1));
  return ret;
}

// trival, since A2G = M2G( A2M )
std::vector<GTy> A2G(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in) {
  auto in_m = A2M(ctx, in);
  return M2G(ctx, in_m);
}

// std::vector<GTy> P2G([[maybe_unused]] std::shared_ptr<Context>& ctx,
//                      absl::Span<const PTy> in) {
//   return std::vector<GTy>(in.size());
// }

}  // namespace mcpsi::internal
