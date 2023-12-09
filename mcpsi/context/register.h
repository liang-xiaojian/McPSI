#pragma once

#include "mcpsi/context/context.h"
#include "mcpsi/context/state.h"
#include "mcpsi/cr/cr.h"
#include "mcpsi/ss/protocol.h"

namespace mcpsi {

void inline SetupContext(std::shared_ptr<Context> ctx) {
  // Generate a same seed
  uint128_t seed = ctx->GetState<Connection>()->SyncSeed();
  // Shared Prg, all parities own a same Prg (with same seed)
  ctx->AddState<Prg>(seed);
  // Create Basic Protocol
  ctx->AddState<Protocol>(ctx);
  // Get SPDZ key
  auto key = ctx->GetState<Protocol>()->GetKey();
  // Create Correlated Randomness Generator
  ctx->AddState<Correlation>(ctx);
  // FIX ME: maybe?
  // 1. ctx->AddState<Correlation>(ctx,key);
  // 2. ctx->GetState<Correlation>()->OneTimeSetup();
  // Set SPDZ key
  ctx->GetState<Correlation>()->SetKey(key);
  // strange !!!
  // But Prf setup need "RandA" (which need correlated randomness)
  // TODO: fix it
  ctx->GetState<Protocol>()->SetupPrf();
}

void inline MockSetupContext(std::vector<std::shared_ptr<Context>>& ctxs) {
  YACL_ENFORCE(ctxs.size() == 2);
  auto task0 = std::async([&] { SetupContext(ctxs[0]); });
  auto task1 = std::async([&] { SetupContext(ctxs[1]); });
  task0.get();
  task1.get();
}

}  // namespace mcpsi
