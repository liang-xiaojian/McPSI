#include "mcpsi/ss/gshare.h"

#include <unordered_set>
#include <vector>

#include "mcpsi/cr/cr.h"
#include "mcpsi/ss/protocol.h"
#include "yacl/base/byte_container_view.h"
#include "yacl/math/mpint/mp_int.h"
#include "yacl/utils/parallel.h"

namespace mcpsi::internal {

// DY-exponent
std::vector<ATy> DyExp(std::shared_ptr<Context> &ctx,
                       absl::Span<const ATy> in) {
  const size_t num = in.size();
  auto prot = ctx->GetState<Protocol>();

  // DY-PRF = g^{1/(k+x)}
  auto prf_k = prot->GetPrfK();  // distributed key for PRF (A-share)
  auto ext_k = std::vector<ATy>(num, prf_k);
  auto add = AddAA(ctx, in, ext_k);
  auto inv = InvA(ctx, add);
  return inv;
}

std::vector<ATy> DyExp_cache(std::shared_ptr<Context> &ctx,
                             absl::Span<const ATy> in) {
  const size_t num = in.size();
  auto prot = ctx->GetState<Protocol>();

  // DY-PRF = g^{1/(k+x)}
  auto prf_k = prot->GetPrfK();  // distributed key for PRF (A-share)
  auto ext_k = std::vector<ATy>(num, prf_k);
  auto add = AddAA_cache(ctx, in, ext_k);
  auto inv = InvA_cache(ctx, add);
  return inv;
}

std::vector<ATy> DyExpGet(std::shared_ptr<Context> &ctx, size_t num) {
  // auto inA = GetA(ctx, num);
  // return DyExp(ctx, inA);
  auto prot = ctx->GetState<Protocol>();
  auto conn = ctx->GetState<Connection>();
  auto cr = ctx->GetState<Correlation>();
  auto [a, b, c, r, prf_k] = cr->DyBeaverTripleGet(num);

  YACL_ENFORCE(prf_k.val == prot->GetPrfK().val);

  auto [b_val, b_mac] = Unpack(b);
  auto [c_val, c_mac] = Unpack(c);

  auto buff = conn->Recv(ctx->NextRank(), "DyExpSetGet");
  auto diff = absl::MakeSpan(reinterpret_cast<PTy *>(buff.data()), num);

  auto new_b_mac = MulPP(ctx, b_mac, diff);
  Pack(absl::MakeConstSpan(b_val), absl::MakeConstSpan(new_b_mac),
       absl::MakeSpan(b));
  prot->AShareBufferAppend(b);

  auto new_c_val = MulPP(ctx, c_val, diff);
  auto new_c_mac = MulPP(ctx, c_mac, diff);

  Pack(absl::MakeConstSpan(new_c_val), absl::MakeConstSpan(new_c_mac),
       absl::MakeSpan(c));

  auto val = AddAA(ctx, r, c);
  auto val_p = A2P(ctx, val);
  auto inv_p = InvP(ctx, val_p);

  return MulAP(ctx, a, inv_p);
}

std::vector<ATy> DyExpGet_cache(std::shared_ptr<Context> &ctx, size_t num) {
  auto cr = ctx->GetState<Correlation>();

  cr->DyBeaverTripleGet_cache(num);
  auto inv_p = ZerosA_cache(ctx, num);
  return inv_p;
}

std::vector<ATy> DyExpSet(std::shared_ptr<Context> &ctx,
                          absl::Span<const PTy> in) {
  // auto inA = SetA(ctx, in);
  const size_t num = in.size();
  auto prot = ctx->GetState<Protocol>();
  auto conn = ctx->GetState<Connection>();
  auto cr = ctx->GetState<Correlation>();
  auto [a, b, c, r, prf_k] = cr->DyBeaverTripleSet(num);

  YACL_ENFORCE(prf_k.val == prot->GetPrfK().val);

  auto [b_val, b_mac] = Unpack(b);
  auto [c_val, c_mac] = Unpack(c);

  auto diff = DivPP(ctx, in, b_val);

  conn->SendAsync(
      conn->NextRank(),
      yacl::ByteContainerView(diff.data(), diff.size() * sizeof(PTy)),
      "DyExpSetGet");

  auto new_b_mac = MulPP(ctx, b_mac, diff);
  Pack(absl::MakeConstSpan(in), absl::MakeConstSpan(new_b_mac),
       absl::MakeSpan(b));
  prot->AShareBufferAppend(b);

  auto new_c_val = MulPP(ctx, c_val, diff);
  auto new_c_mac = MulPP(ctx, c_mac, diff);

  Pack(absl::MakeConstSpan(new_c_val), absl::MakeConstSpan(new_c_mac),
       absl::MakeSpan(c));

  auto val = AddAA(ctx, r, c);
  auto val_p = A2P(ctx, val);
  auto inv_p = InvP(ctx, val_p);

  return MulAP(ctx, a, inv_p);
  // return DyExp(ctx, inA);
}

std::vector<ATy> DyExpSet_cache(std::shared_ptr<Context> &ctx,
                                absl::Span<const PTy> in) {
  const size_t num = in.size();
  auto cr = ctx->GetState<Correlation>();
  cr->DyBeaverTripleSet_cache(num);
  auto zeros = ZerosA_cache(ctx, num);

  return MulAP(ctx, zeros, in);
}

// fair-DY-exponent
std::vector<ATy> ScalarDyExp(std::shared_ptr<Context> &ctx, const ATy &scalar,
                             absl::Span<const ATy> in) {
  const size_t num = in.size();
  auto prot = ctx->GetState<Protocol>();
  auto prf_k = prot->GetPrfK();  // distributed key for PRF (A-share)

  // in + k
  auto ext_k = std::vector<ATy>(num, prf_k);
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
  return MulAA(ctx, absl::MakeConstSpan(r),
               absl::MakeConstSpan(scalar_inv_pub));
}

std::vector<ATy> ScalarDyExp_cache(std::shared_ptr<Context> &ctx,
                                   const ATy &scalar,
                                   absl::Span<const ATy> in) {
  const size_t num = in.size();
  auto prot = ctx->GetState<Protocol>();
  auto prf_k = prot->GetPrfK();  // distributed key for PRF (A-share)

  // in + k
  auto ext_k = std::vector<ATy>(num, prf_k);
  auto add = AddAA_cache(ctx, in, ext_k);
  // scalar / (in + k)
  auto r = RandA_cache(ctx, num);
  // r * in
  auto mul = MulAA_cache(ctx, absl::MakeConstSpan(r), absl::MakeConstSpan(add));
  // reveal r * in
  auto pub = A2P_cache(ctx, absl::MakeConstSpan(mul));
  // inv = (r * in)^{-1}
  auto inv_pub = InvP_cache(ctx, absl::MakeConstSpan(pub));
  auto scalar_inv_pub = ScalarMulAP_cache(ctx, scalar, inv_pub);
  return MulAA_cache(ctx, absl::MakeConstSpan(r),
                     absl::MakeConstSpan(scalar_inv_pub));
}

std::vector<ATy> ScalarDyExpGet(std::shared_ptr<Context> &ctx,
                                const ATy &scalar, size_t num) {
  auto inA = GetA(ctx, num);
  return ScalarDyExp(ctx, scalar, inA);
}
std::vector<ATy> ScalarDyExpGet_cache(std::shared_ptr<Context> &ctx,
                                      const ATy &scalar, size_t num) {
  auto inA = GetA_cache(ctx, num);
  return ScalarDyExp_cache(ctx, scalar, inA);
}

std::vector<ATy> ScalarDyExpSet(std::shared_ptr<Context> &ctx,
                                const ATy &scalar, absl::Span<const PTy> in) {
  auto inA = SetA(ctx, in);
  return ScalarDyExp(ctx, scalar, inA);
}
std::vector<ATy> ScalarDyExpSet_cache(std::shared_ptr<Context> &ctx,
                                      const ATy &scalar,
                                      absl::Span<const PTy> in) {
  auto inA = SetA_cache(ctx, in);
  return ScalarDyExp_cache(ctx, scalar, inA);
}

std::vector<MTy> A2M(std::shared_ptr<Context> &ctx, absl::Span<const ATy> in) {
  const size_t num = in.size();
  auto prot = ctx->GetState<Protocol>();
  auto Ggroup = prot->GetGroup();

  auto ret = std::vector<MTy>(num);
  // Unpack by hand

  yacl::parallel_for(0, num, [&](uint64_t bg, uint64_t ed) {
    for (auto i = bg; i < ed; ++i) {
      ret[i].val = Ggroup->MulBase(ym::MPInt(in[i].val.GetVal()));
      ret[i].mac = Ggroup->MulBase(ym::MPInt(in[i].mac.GetVal()));
    }
  });

  // for (size_t i = 0; i < num; ++i) {
  //   ret[i].val = Ggroup->MulBase(ym::MPInt(inv[i].val.GetVal()));
  //   ret[i].mac = Ggroup->MulBase(ym::MPInt(inv[i].mac.GetVal()));
  // }
  return ret;
}

std::vector<MTy> A2M_cache([[maybe_unused]] std::shared_ptr<Context> &ctx,
                           absl::Span<const ATy> in) {
  const size_t num = in.size();
  return std::vector<MTy>(num);
}

std::vector<GTy> M2G(std::shared_ptr<Context> &ctx, absl::Span<const MTy> in) {
  const size_t num = in.size();
  auto spdz_key = ctx->GetState<Protocol>()->GetKey();

  auto prot = ctx->GetState<Protocol>();
  auto Ggroup = prot->GetGroup();
  auto prf_zero = Ggroup->Sub(Ggroup->GetGenerator(), Ggroup->GetGenerator());
  // auto prf_g = prot->GetPrfG();  // generator for PRF

  std::vector<GTy> ret(num);

  auto GTy_size = Ggroup->GetSerializeLength(kOctetFormat);
  yacl::Buffer send_buf = yacl::Buffer(GTy_size * num);

  yacl::parallel_for(0, num, [&](uint64_t bg, uint64_t ed) {
    for (auto i = bg; i < ed; ++i) {
      Ggroup->SerializePoint(in[i].val, kOctetFormat,
                             send_buf.data<uint8_t>() + i * GTy_size, GTy_size);
    }
  });
  // for (size_t i = 0; i < num; ++i) {
  //   Ggroup->SerializePoint(in[i].val, kOctetFormat,
  //                          send_buf.data<uint8_t>() + i * GTy_size,
  //                          GTy_size);
  // }

  auto conn = ctx->GetConnection();
  yacl::Buffer buf;

  if (ctx->GetRank() == 0) {
    conn->SendAsync(ctx->NextRank(), send_buf, "M2G:0");
    buf = conn->Recv(ctx->NextRank(), "M2G:1");
    YACL_ENFORCE(buf.size() == send_buf.size());
  } else {
    buf = conn->Recv(ctx->NextRank(), "M2G:0");
    conn->SendAsync(ctx->NextRank(), send_buf, "M2G:1");
    YACL_ENFORCE(buf.size() == send_buf.size());
  }

  yacl::parallel_for(0, num, [&](uint64_t bg, uint64_t ed) {
    for (auto i = bg; i < ed; ++i) {
      ret[i] = Ggroup->DeserializePoint(
          {buf.data<uint8_t>() + i * GTy_size, GTy_size}, kOctetFormat);
      Ggroup->AddInplace(&ret[i], in[i].val);
    }
  });

  // for (size_t i = 0; i < num; ++i) {
  //   ret[i] =
  //       Ggroup->DeserializePoint({buf.data<uint8_t>() + i * GTy_size,
  //       GTy_size},
  //                                kOctetFormat);
  //   Ggroup->AddInplace(&ret[i], in[i].val);
  // }

  auto sync_seed = conn->SyncSeed();
  auto coef = op::Rand(sync_seed, num);

  GTy real_val_affine = Ggroup->CopyPoint(prf_zero);  // zero
  GTy mac_affine = Ggroup->CopyPoint(prf_zero);       // zero

  // compute the linear combination by hand
  // for (size_t i = 0; i < num; ++i) {
  //   auto tmp_val = Ggroup->Mul(ret[i], ym::MPInt(coef[i].GetVal()));
  //   real_val_affine = Ggroup->Add(real_val_affine, tmp_val);

  //   auto tmp_mac = Ggroup->Mul(in[i].mac, ym::MPInt(coef[i].GetVal()));
  //   mac_affine = Ggroup->Add(mac_affine, tmp_mac);
  // }

  // [Warning] Low Efficency??? Why
  //
  // auto [real_val_affine, mac_affine] = yacl::parallel_reduce<
  //     std::pair<GTy, GTy>,
  //     std::function<std::pair<GTy, GTy>(uint64_t, uint64_t)>,
  //     std::function<std::pair<GTy, GTy>(const std::pair<GTy, GTy> &,
  //                                       const std::pair<GTy, GTy> &)>>(
  //     0, num, 256,
  //     [&](uint64_t bg, uint64_t ed) {
  //       auto real_val_affine = Ggroup->CopyPoint(prf_zero);
  //       auto mac_affine = Ggroup->CopyPoint(prf_zero);

  //       for (auto i = bg; i < ed; ++i) {
  //         auto mp_ceof_i = ym::MPInt(coef[i].GetVal());
  //         auto tmp_val = Ggroup->Mul(ret[i], mp_ceof_i);
  //         auto tmp_mac = Ggroup->Mul(in[i].mac, mp_ceof_i);

  //         Ggroup->AddInplace(&real_val_affine, tmp_val);
  //         Ggroup->AddInplace(&mac_affine, tmp_mac);
  //       }
  //       return std::make_pair(real_val_affine, mac_affine);
  //     },
  //     [&Ggroup](const std::pair<GTy, GTy> &lhs,
  //               const std::pair<GTy, GTy> &rhs) {
  //       auto first = Ggroup->Add(lhs.first, rhs.first);
  //       auto second = Ggroup->Add(lhs.second, rhs.second);
  //       return std::make_pair(first, second);
  //     });

  size_t num_threads = yacl::get_num_threads();
  std::vector<GTy> real_val_affine_vec(2 * num_threads);
  std::vector<GTy> mac_affine_vec(2 * num_threads);
  // init
  for (size_t i = 0; i < 2 * num_threads; ++i) {
    real_val_affine_vec[i] = Ggroup->CopyPoint(prf_zero);
    mac_affine_vec[i] = Ggroup->CopyPoint(prf_zero);
  }

  yacl::parallel_for(0, num, [&](uint64_t bg, uint64_t ed) {
    auto size = ed - bg;
    auto offset = bg / size;

    // to avoid overflow
    if (offset > num_threads) {
      offset = num_threads + 1;
    }

    auto val_affine_addr = &real_val_affine_vec[offset];
    auto mac_affine_addr = &mac_affine_vec[offset];
    for (auto i = bg; i < ed; ++i) {
      auto mp_ceof_i = ym::MPInt(coef[i].GetVal());
      auto tmp_val = Ggroup->Mul(ret[i], mp_ceof_i);
      auto tmp_mac = Ggroup->Mul(in[i].mac, mp_ceof_i);

      Ggroup->AddInplace(val_affine_addr, tmp_val);
      Ggroup->AddInplace(mac_affine_addr, tmp_mac);
    }
  });

  for (size_t i = 0; i < 2 * num_threads; ++i) {
    if (!Ggroup->PointEqual(prf_zero, real_val_affine_vec[i])) {
      Ggroup->AddInplace(&real_val_affine, real_val_affine_vec[i]);
      Ggroup->AddInplace(&mac_affine, mac_affine_vec[i]);
    }
  }

  auto local_mac_GTy =
      Ggroup->Mul(real_val_affine, ym::MPInt(spdz_key.GetVal()));

  auto zero_mac_GTy = Ggroup->Sub(mac_affine, local_mac_GTy);

  yacl::Buffer zero_mac_buf(GTy_size);
  Ggroup->SerializePoint(zero_mac_GTy, kOctetFormat,
                         zero_mac_buf.data<uint8_t>(), GTy_size);

  auto remote_mac_buf = conn->ExchangeWithCommit(zero_mac_buf);
  GTy remote_mac_GTy = Ggroup->DeserializePoint(
      {remote_mac_buf.data<uint8_t>(), GTy_size}, kOctetFormat);

  YACL_ENFORCE(
      Ggroup->PointEqual(Ggroup->Add(zero_mac_GTy, remote_mac_GTy), prf_zero));

  return ret;
}

std::vector<GTy> M2G_cache([[maybe_unused]] std::shared_ptr<Context> &ctx,
                           absl::Span<const MTy> in) {
  const size_t num = in.size();
  return std::vector<GTy>(num);
}

// trival, since A2G = M2G( A2M )
std::vector<GTy> A2G(std::shared_ptr<Context> &ctx, absl::Span<const ATy> in) {
  auto in_m = A2M(ctx, in);
  return M2G(ctx, in_m);
}

std::vector<GTy> A2G_cache(std::shared_ptr<Context> &ctx,
                           absl::Span<const ATy> in) {
  auto in_m = A2M_cache(ctx, in);
  return M2G_cache(ctx, in_m);
}

// DY-OPRF
std::vector<GTy> DyOprf(std::shared_ptr<Context> &ctx,
                        absl::Span<const ATy> in) {
  auto dy_exp = DyExp(ctx, in);
  return A2G(ctx, dy_exp);
}
std::vector<GTy> DyOprf_cache(std::shared_ptr<Context> &ctx,
                              absl::Span<const ATy> in) {
  auto dy_exp = DyExp_cache(ctx, in);
  return A2G_cache(ctx, dy_exp);
}

std::vector<GTy> DyOprfGet(std::shared_ptr<Context> &ctx, size_t num) {
  auto dy_exp = DyExpGet(ctx, num);
  return A2G(ctx, dy_exp);
}
std::vector<GTy> DyOprfGet_cache(std::shared_ptr<Context> &ctx, size_t num) {
  auto dy_exp = DyExpGet_cache(ctx, num);
  return A2G_cache(ctx, dy_exp);
}

std::vector<GTy> DyOprfSet(std::shared_ptr<Context> &ctx,
                           absl::Span<const PTy> in) {
  auto dy_exp = DyExpSet(ctx, in);
  return A2G(ctx, dy_exp);
}
std::vector<GTy> DyOprfSet_cache(std::shared_ptr<Context> &ctx,
                                 absl::Span<const PTy> in) {
  auto dy_exp = DyExpSet_cache(ctx, in);
  return A2G_cache(ctx, dy_exp);
}

// Scalar-DY-OPRF
std::vector<GTy> ScalarDyOprf(std::shared_ptr<Context> &ctx, const ATy &scalar,
                              absl::Span<const ATy> in) {
  auto scalar_dy_exp = ScalarDyExp(ctx, scalar, in);
  return A2G(ctx, scalar_dy_exp);
}

std::vector<GTy> ScalarDyOprf_cache(std::shared_ptr<Context> &ctx,
                                    const ATy &scalar,
                                    absl::Span<const ATy> in) {
  auto scalar_dy_exp = ScalarDyExp_cache(ctx, scalar, in);
  return A2G_cache(ctx, scalar_dy_exp);
}

std::vector<GTy> ScalarDyOprfGet(std::shared_ptr<Context> &ctx,
                                 const ATy &scalar, size_t num) {
  auto scalar_dy_exp = ScalarDyExpGet(ctx, scalar, num);
  return A2G(ctx, scalar_dy_exp);
}

std::vector<GTy> ScalarDyOprfGet_cache(std::shared_ptr<Context> &ctx,
                                       const ATy &scalar, size_t num) {
  auto scalar_dy_exp = ScalarDyExpGet_cache(ctx, scalar, num);
  return A2G_cache(ctx, scalar_dy_exp);
}

std::vector<GTy> ScalarDyOprfSet(std::shared_ptr<Context> &ctx,
                                 const ATy &scalar, absl::Span<const PTy> in) {
  auto scalar_dy_exp = ScalarDyExpSet(ctx, scalar, in);
  return A2G(ctx, scalar_dy_exp);
}

std::vector<GTy> ScalarDyOprfSet_cache(std::shared_ptr<Context> &ctx,
                                       const ATy &scalar,
                                       absl::Span<const PTy> in) {
  auto scalar_dy_exp = ScalarDyExpSet_cache(ctx, scalar, in);
  return A2G_cache(ctx, scalar_dy_exp);
}

std::vector<ATy> CPSI(std::shared_ptr<Context> &ctx, absl::Span<const ATy> set0,
                      absl::Span<const ATy> set1, absl::Span<const ATy> data) {
  YACL_ENFORCE(set1.size() == data.size());
  auto prot = ctx->GetState<Protocol>();
  auto Ggroup = prot->GetGroup();

  auto shuffle0 = ShuffleA(ctx, set0);
  auto _shuffle_tmp = ShuffleA(ctx, set1, data);
  auto &shuffle1 = _shuffle_tmp[0];
  auto &shuffle_data = _shuffle_tmp[1];

  auto reveal0 = DyOprf(ctx, shuffle0);
  auto reveal1 = DyOprf(ctx, shuffle1);

  auto group_hash = [&Ggroup](const GTy &val) {
    return Ggroup->HashPoint(val);
  };
  auto group_equal = [&Ggroup](const GTy &lhs, const GTy &rhs) {
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

  auto selected_data = FilterA(ctx, absl::MakeConstSpan(shuffle_data),
                               absl::MakeConstSpan(indexes));
  return selected_data;
}

std::vector<ATy> CPSI_cache(std::shared_ptr<Context> &ctx,
                            absl::Span<const ATy> set0,
                            absl::Span<const ATy> set1,
                            absl::Span<const ATy> data) {
  YACL_ENFORCE(set1.size() == data.size());

  auto shuffle0 = ShuffleA_cache(ctx, set0);
  auto _shuffle_tmp = ShuffleA_cache(ctx, set1, data);
  auto &shuffle1 = _shuffle_tmp[0];
  auto &shuffle_data = _shuffle_tmp[1];

  [[maybe_unused]] auto reveal0 = DyOprf_cache(ctx, shuffle0);
  [[maybe_unused]] auto reveal1 = DyOprf_cache(ctx, shuffle1);

  std::vector<size_t> indexes(data.size());

  auto selected_data = FilterA_cache(ctx, absl::MakeConstSpan(shuffle_data),
                                     absl::MakeConstSpan(indexes));
  return selected_data;
}

std::vector<ATy> FairCPSI(std::shared_ptr<Context> &ctx,
                          absl::Span<const ATy> set0,
                          absl::Span<const ATy> set1,
                          absl::Span<const ATy> data) {
  YACL_ENFORCE(set1.size() == data.size());
  auto prot = ctx->GetState<Protocol>();
  auto Ggroup = prot->GetGroup();

  auto shuffle0 = ShuffleA(ctx, set0);
  auto _shuffle_tmp = ShuffleA(ctx, set1, data);
  auto &shuffle1 = _shuffle_tmp[0];
  auto &shuffle_data = _shuffle_tmp[1];

  auto [scalar_a, bits] = RandFairA(ctx, 1);
  auto reveal0 = ScalarDyOprf(ctx, scalar_a[0], shuffle0);
  auto reveal1 = DyOprf(ctx, shuffle1);

  auto scalar_p = FairA2P(ctx, scalar_a, bits);
  auto scalar_mp = ym::MPInt(scalar_p[0].GetVal());
  for (size_t i = 0; i < reveal1.size(); ++i) {
    Ggroup->MulInplace(&reveal1[i], scalar_mp);
  }

  auto group_hash = [&Ggroup](const GTy &val) {
    return Ggroup->HashPoint(val);
  };
  auto group_equal = [&Ggroup](const GTy &lhs, const GTy &rhs) {
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

  auto selected_data = FilterA(ctx, absl::MakeConstSpan(shuffle_data),
                               absl::MakeConstSpan(indexes));
  return selected_data;
}

std::vector<ATy> FairCPSI_cache(std::shared_ptr<Context> &ctx,
                                absl::Span<const ATy> set0,
                                absl::Span<const ATy> set1,
                                absl::Span<const ATy> data) {
  YACL_ENFORCE(set1.size() == data.size());

  auto shuffle0 = ShuffleA_cache(ctx, set0);
  auto _shuffle_tmp = ShuffleA_cache(ctx, set1, data);
  auto &shuffle1 = _shuffle_tmp[0];
  auto &shuffle_data = _shuffle_tmp[1];

  auto [scalar_a, bits] = RandFairA_cache(ctx, 1);
  [[maybe_unused]] auto reveal0 =
      ScalarDyOprf_cache(ctx, scalar_a[0], shuffle0);
  [[maybe_unused]] auto reveal1 = DyOprf_cache(ctx, shuffle1);
  [[maybe_unused]] auto scalar_p = FairA2P_cache(ctx, scalar_a, bits);

  std::vector<size_t> indexes(data.size());

  auto selected_data = FilterA_cache(ctx, absl::MakeConstSpan(shuffle_data),
                                     absl::MakeConstSpan(indexes));
  return selected_data;
}

}  // namespace mcpsi::internal
