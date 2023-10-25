#include "test/ss/gshare.h"

#include "test/ss/protocol.h"
#include "yacl/base/byte_container_view.h"
#include "yacl/math/mpint/mp_int.h"

namespace test::internal {

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

  // FIX ME:
  // sample coefficient before open ( I guess )
  auto coef = RandP(ctx, num);

  auto lctx = ctx->GetLink();
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

  auto u64_coef = absl::MakeSpan(reinterpret_cast<uint64_t*>(coef.data()), num);
  GTy real_val_affine{1};
  GTy mac_affine{1};

  for (size_t i = 0; i < num; ++i) {
    real_val_affine = real_val_affine.MulMod(
        ret[i].PowMod(ym::MPInt(u64_coef[i]), prf_mod), prf_mod);
    mac_affine = mac_affine.MulMod(
        in[i].mac.PowMod(ym::MPInt(u64_coef[i]), prf_mod), prf_mod);
  }

  auto local_mac_GTy =
      real_val_affine.PowMod(ym::MPInt(spdz_key.GetVal()), prf_mod);
  std::array<uint64_t, 2> local_mac;
  local_mac[0] = local_mac_GTy.Get<uint64_t>();
  local_mac[1] = mac_affine.Get<uint64_t>();
  std::array<uint64_t, 2> remote_mac;
  if (ctx->GetRank() == 0) {
    lctx->SendAsync(
        ctx->NextRank(),
        yacl::ByteContainerView(local_mac.data(), 2 * sizeof(uint64_t)),
        "M2G:0");
    auto buf = lctx->Recv(ctx->NextRank(), "M2G:1");
    memcpy(remote_mac.data(), buf.data(), buf.size());

  } else {
    auto buf = lctx->Recv(ctx->NextRank(), "M2G:0");
    lctx->SendAsync(
        ctx->NextRank(),
        yacl::ByteContainerView(local_mac.data(), 2 * sizeof(uint64_t)),
        "M2G:1");
    memcpy(remote_mac.data(), buf.data(), buf.size());
  }

  GTy check_mac = local_mac_GTy.MulMod(ym::MPInt(remote_mac[0]), prf_mod);
  GTy final_mac = mac_affine.MulMod(ym::MPInt(remote_mac[1]), prf_mod);

  YACL_ENFORCE(check_mac == final_mac);

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

}  // namespace test::internal
