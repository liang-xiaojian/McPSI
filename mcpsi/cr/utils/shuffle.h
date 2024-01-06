#pragma once

#include "mcpsi/context/state.h"
#include "mcpsi/cr/utils/ot_adapter.h"
#include "mcpsi/ss/type.h"

namespace mcpsi::shuffle {

void ShuffleSend(std::shared_ptr<Connection>& conn,
                 std::shared_ptr<ot::OtAdapter>& ot_ptr,
                 absl::Span<const size_t> perm, absl::Span<internal::PTy> delta,
                 size_t repeat = 1);

void ShuffleRecv(std::shared_ptr<Connection> conn,
                 std::shared_ptr<ot::OtAdapter>& ot_ptr,
                 absl::Span<internal::PTy> a, absl::Span<internal::PTy> b,
                 size_t repeat = 1);

}  // namespace mcpsi::shuffle
