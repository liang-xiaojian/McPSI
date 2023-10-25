#pragma once

#include "test/context/context.h"
#include "test/context/state.h"
#include "test/cr/fake_cr.h"
#include "test/ss/protocol.h"

namespace test {

void inline InitContext(std::shared_ptr<Context> ctx) {
  uint128_t seed = ctx->GetState<Connection>()->SyncSeed();
  ctx->states_->AddState<Prg>(seed);
  ctx->states_->AddState<Protocol>(ctx);
  auto key = ctx->GetState<Protocol>()->GetKey();
  ctx->states_->AddState<FakeCorrelation>(ctx);
  ctx->GetState<FakeCorrelation>()->SetKey(key);
}

void inline MockInitContext(std::vector<std::shared_ptr<Context>>& ctxs) {
  YACL_ENFORCE(ctxs.size() == 2);
  auto task0 = std::async([&] { InitContext(ctxs[0]); });
  auto task1 = std::async([&] { InitContext(ctxs[1]); });
  task0.get();
  task1.get();
}

}  // namespace test
