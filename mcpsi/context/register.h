#pragma once

#include "mcpsi/context/context.h"
#include "mcpsi/context/state.h"
#include "mcpsi/cr/fake_cr.h"
#include "mcpsi/ss/protocol.h"

namespace mcpsi {

void inline InitContext(std::shared_ptr<Context> ctx) {
  // Generate a same seed
  uint128_t seed = ctx->GetState<Connection>()->SyncSeed();
  // Shared Prg, all parities own a same Prg (with same seed)
  ctx->AddState<Prg>(seed);
  // Create Basic Protocol
  ctx->AddState<Protocol>(ctx);
  // Get SPDZ key
  auto key = ctx->GetState<Protocol>()->GetKey();
  // Create Correlated Randomness Generator
  ctx->AddState<FakeCorrelation>(ctx);
  // Set SPDZ key
  ctx->GetState<FakeCorrelation>()->SetKey(key);
  // strange !!!
  // But Prf setup need "RandA" (which need correlated randomness)
  // TODO: fix it
  ctx->GetState<Protocol>()->SetupPrf();
}

void inline MockInitContext(std::vector<std::shared_ptr<Context>>& ctxs) {
  YACL_ENFORCE(ctxs.size() == 2);
  auto task0 = std::async([&] { InitContext(ctxs[0]); });
  auto task1 = std::async([&] { InitContext(ctxs[1]); });
  task0.get();
  task1.get();
}

}  // namespace mcpsi
