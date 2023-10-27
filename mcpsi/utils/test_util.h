#pragma once

#include "mcpsi/context/context.h"
#include "yacl/link/test_util.h"

namespace test {
std::vector<std::shared_ptr<Context>> MockContext(size_t world_size) {
  auto lctxs = yacl::link::test::SetupWorld(fmt::format("sim.{}", world_size),
                                            world_size);
  std::vector<std::shared_ptr<Context>> result;
  for (uint32_t rank = 0; rank < world_size; ++rank) {
    result.emplace_back(std::make_shared<Context>(lctxs[rank]));
  }
  return result;
}
};  // namespace test
