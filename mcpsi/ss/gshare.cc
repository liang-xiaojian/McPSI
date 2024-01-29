#include "mcpsi/ss/gshare.h"

#include <unordered_set>

#include "mcpsi/ss/protocol.h"
#include "yacl/base/byte_container_view.h"
#include "yacl/math/mpint/mp_int.h"

namespace mcpsi::internal {

std::vector<MTy> A2M(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in) {
  const size_t num = in.size();

  auto prot = ctx->GetState<Protocol>();
  // auto prf_g = prot->GetPrfG();  // generator for PRF
  auto prf_k = prot->GetPrfK();  // distributed key for PRF (A-share)
  auto Ggroup = prot->GetGroup();
  auto ext_k = std::vector<ATy>(num, prf_k);
  // in + k
  auto add = AddAA(ctx, in, ext_k);
  // 1 / (in + k)
  auto inv = InvA(ctx, add);

  auto ret = std::vector<MTy>(num);
  // Unpack by hand
  for (size_t i = 0; i < num; ++i) {
    ret[i].val = Ggroup->MulBase(ym::MPInt(inv[i].val.GetVal()));
    ret[i].mac = Ggroup->MulBase(ym::MPInt(inv[i].mac.GetVal()));
  }
  return ret;
}

std::vector<MTy> A2M_cache(std::shared_ptr<Context>& ctx,
                           absl::Span<const ATy> in) {
  const size_t num = in.size();

  auto ext_k = std::vector<ATy>(num);
  auto add = AddAA_cache(ctx, in, ext_k);
  [[maybe_unused]] auto inv = InvA_cache(ctx, add);
  return std::vector<MTy>(num);
}

std::vector<MTy> ScalarA2M(std::shared_ptr<Context>& ctx, const ATy& scalar,
                           absl::Span<const ATy> in) {
  const size_t num = in.size();

  auto prot = ctx->GetState<Protocol>();
  // auto prf_g = prot->GetPrfG();  // generator for PRF
  auto prf_k = prot->GetPrfK();  // distributed key for PRF (A-share)
  auto Ggroup = prot->GetGroup();
  auto ext_k = std::vector<ATy>(num, prf_k);
  // in + k
  auto add = AddAA(ctx, in, ext_k);
  // scalar / (in + k)

  auto r = RandA(ctx, num);
  // r * in
  auto mul = MulAA(ctx, absl::MakeConstSpan(r), absl::MakeConstSpan(add));
  // reveal r * in
  auto pub = A2P(ctx, absl::MakeConstSpan(mul));
  // inv = (r * in)^{-1}
  auto inv_pub = InvP(ctx, absl::MakeConstSpan(pub));
  auto scalar_inv_pub = ScalarMulAP(ctx, scalar, inv_pub);
  auto scalar_inv =
      MulAA(ctx, absl::MakeConstSpan(r), absl::MakeConstSpan(scalar_inv_pub));

  auto ret = std::vector<MTy>(num);
  // Unpack by hand
  for (size_t i = 0; i < num; ++i) {
    ret[i].val = Ggroup->MulBase(ym::MPInt(scalar_inv[i].val.GetVal()));
    ret[i].mac = Ggroup->MulBase(ym::MPInt(scalar_inv[i].mac.GetVal()));
  }
  return ret;
}

std::vector<MTy> ScalarA2M_cache(std::shared_ptr<Context>& ctx,
                                 const ATy& scalar, absl::Span<const ATy> in) {
  const size_t num = in.size();

  auto ext_k = std::vector<ATy>(num);
  // in + k
  auto add = AddAA_cache(ctx, in, ext_k);
  // scalar / (in + k)

  auto r = RandA_cache(ctx, num);
  // r * in
  auto mul = MulAA_cache(ctx, absl::MakeConstSpan(r), absl::MakeConstSpan(in));
  // reveal r * in
  auto pub = A2P_cache(ctx, absl::MakeConstSpan(mul));
  // inv = (r * in)^{-1}
  auto inv_pub = InvP_cache(ctx, absl::MakeConstSpan(pub));
  auto scalar_inv_pub = ScalarMulAP_cache(ctx, scalar, inv_pub);
  [[maybe_unused]] auto scalar_inv = MulAA_cache(
      ctx, absl::MakeConstSpan(r), absl::MakeConstSpan(scalar_inv_pub));

  return std::vector<MTy>(num);
}

std::vector<GTy> M2G(std::shared_ptr<Context>& ctx, absl::Span<const MTy> in) {
  const size_t num = in.size();
  auto spdz_key = ctx->GetState<Protocol>()->GetKey();

  auto prot = ctx->GetState<Protocol>();
  auto Ggroup = prot->GetGroup();
  auto prf_zero = Ggroup->Sub(Ggroup->GetGenerator(), Ggroup->GetGenerator());
  // auto prf_g = prot->GetPrfG();  // generator for PRF

  std::vector<GTy> ret(num);

  auto GTy_size =
      Ggroup->GetSerializeLength(yc::PointOctetFormat::X962Compressed);
  yacl::Buffer send_buf = yacl::Buffer(GTy_size * num);
  for (size_t i = 0; i < num; ++i) {
    Ggroup->SerializePoint(in[i].val, yc::PointOctetFormat::X962Compressed,
                           send_buf.data<uint8_t>() + i * GTy_size, GTy_size);
  }

  auto conn = ctx->GetConnection();
  if (ctx->GetRank() == 0) {
    conn->SendAsync(ctx->NextRank(), send_buf, "M2G:0");
    auto buf = conn->Recv(ctx->NextRank(), "M2G:1");
    YACL_ENFORCE(buf.size() == send_buf.size());
    for (size_t i = 0; i < num; ++i) {
      ret[i] = Ggroup->Add(in[i].val,
                           Ggroup->DeserializePoint(
                               {buf.data<uint8_t>() + i * GTy_size, GTy_size},
                               yc::PointOctetFormat::X962Compressed));
    }
  } else {
    auto buf = conn->Recv(ctx->NextRank(), "M2G:0");
    conn->SendAsync(ctx->NextRank(), send_buf, "M2G:1");
    YACL_ENFORCE(buf.size() == send_buf.size());
    for (size_t i = 0; i < num; ++i) {
      ret[i] = Ggroup->Add(in[i].val,
                           Ggroup->DeserializePoint(
                               {buf.data<uint8_t>() + i * GTy_size, GTy_size},
                               yc::PointOctetFormat::X962Compressed));
    }
  }
  auto sync_seed = conn->SyncSeed();
  auto coef = op::Rand(sync_seed, num);

  GTy real_val_affine = Ggroup->CopyPoint(prf_zero);  // zero
  GTy mac_affine = Ggroup->CopyPoint(prf_zero);       // zero

  // compute the linear combination by hand
  for (size_t i = 0; i < num; ++i) {
    auto tmp_val = Ggroup->Mul(ret[i], ym::MPInt(coef[i].GetVal()));
    real_val_affine = Ggroup->Add(real_val_affine, tmp_val);

    auto tmp_mac = Ggroup->Mul(in[i].mac, ym::MPInt(coef[i].GetVal()));
    mac_affine = Ggroup->Add(mac_affine, tmp_mac);
  }

  auto local_mac_GTy =
      Ggroup->Mul(real_val_affine, ym::MPInt(spdz_key.GetVal()));

  auto zero_mac_GTy = Ggroup->Sub(mac_affine, local_mac_GTy);

  yacl::Buffer zero_mac_buf(GTy_size);
  Ggroup->SerializePoint(zero_mac_GTy, yc::PointOctetFormat::X962Compressed,
                         zero_mac_buf.data<uint8_t>(), GTy_size);

  auto remote_mac_buf = conn->ExchangeWithCommit(zero_mac_buf);
  GTy remote_mac_GTy =
      Ggroup->DeserializePoint({remote_mac_buf.data<uint8_t>(), GTy_size},
                               yc::PointOctetFormat::X962Compressed);

  YACL_ENFORCE(
      Ggroup->PointEqual(Ggroup->Add(zero_mac_GTy, remote_mac_GTy), prf_zero));

  return ret;
}

std::vector<GTy> M2G_cache([[maybe_unused]] std::shared_ptr<Context>& ctx,
                           absl::Span<const MTy> in) {
  const size_t num = in.size();
  return std::vector<GTy>(num);
}

// trival, since A2G = M2G( A2M )
std::vector<GTy> A2G(std::shared_ptr<Context>& ctx, absl::Span<const ATy> in) {
  auto in_m = A2M(ctx, in);
  return M2G(ctx, in_m);
}

std::vector<GTy> A2G_cache(std::shared_ptr<Context>& ctx,
                           absl::Span<const ATy> in) {
  auto in_m = A2M_cache(ctx, in);
  return M2G_cache(ctx, in_m);
}

std::vector<GTy> ScalarA2G(std::shared_ptr<Context>& ctx, const ATy& scalar,
                           absl::Span<const ATy> in) {
  auto in_m = ScalarA2M(ctx, scalar, in);
  return M2G(ctx, in_m);
}

std::vector<GTy> ScalarA2G_cache(std::shared_ptr<Context>& ctx,
                                 const ATy& scalar, absl::Span<const ATy> in) {
  auto in_m = ScalarA2M_cache(ctx, scalar, in);
  return M2G_cache(ctx, in_m);
}

// std::vector<GTy> P2G([[maybe_unused]] std::shared_ptr<Context>& ctx,
//                      absl::Span<const PTy> in) {
//   return std::vector<GTy>(in.size());
// }

std::vector<ATy> CPSI(std::shared_ptr<Context>& ctx, absl::Span<const ATy> set0,
                      absl::Span<const ATy> set1, absl::Span<const ATy> data) {
  YACL_ENFORCE(set1.size() == data.size());
  auto prot = ctx->GetState<Protocol>();
  auto Ggroup = prot->GetGroup();

  auto shuffle0 = prot->ShuffleA(set0);
  auto _shuffle_tmp = prot->ShuffleA(set1, data);
  auto& shuffle1 = _shuffle_tmp[0];
  auto& shuffle_data = _shuffle_tmp[1];

  auto reveal0 = prot->A2G(shuffle0);
  auto reveal1 = prot->A2G(shuffle1);

  auto group_hash = [&Ggroup](const GTy& val) {
    return Ggroup->HashPoint(val);
  };
  auto group_equal = [&Ggroup](const GTy& lhs, const GTy& rhs) {
    return Ggroup->PointEqual(lhs, rhs);
  };

  std::unordered_set<GTy, decltype(group_hash), decltype(group_equal)> lhs(
      reveal0.begin(), reveal0.end(), 2, group_hash, group_equal);

  std::vector<size_t> indexes;
  for (size_t i = 0; i < reveal1.size(); ++i) {
    if (lhs.count(reveal1[i])) {
      indexes.emplace_back(i);
    }
  }

  auto selected_data = prot->FilterA(absl::MakeConstSpan(shuffle_data),
                                     absl::MakeConstSpan(indexes));
  return selected_data;
}

std::vector<ATy> CPSI_cache(std::shared_ptr<Context>& ctx,
                            absl::Span<const ATy> set0,
                            absl::Span<const ATy> set1,
                            absl::Span<const ATy> data) {
  YACL_ENFORCE(set1.size() == data.size());

  auto shuffle0 = ShuffleA_cache(ctx, set0);
  auto _shuffle_tmp = ShuffleA_cache(ctx, set1, data);
  auto& shuffle1 = _shuffle_tmp[0];
  auto& shuffle_data = _shuffle_tmp[1];

  [[maybe_unused]] auto reveal0 = A2G_cache(ctx, shuffle0);
  [[maybe_unused]] auto reveal1 = A2G_cache(ctx, shuffle1);

  std::vector<size_t> indexes(data.size());

  auto selected_data = FilterA_cache(ctx, absl::MakeConstSpan(shuffle_data),
                                     absl::MakeConstSpan(indexes));
  return selected_data;
}
}  // namespace mcpsi::internal
