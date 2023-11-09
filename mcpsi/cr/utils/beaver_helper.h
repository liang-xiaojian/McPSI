#pragma once

#include "mcpsi/context/state.h"
#include "mcpsi/cr/utils/ot_adapter.h"
#include "mcpsi/ss/type.h"

namespace mcpsi::ot {

class BeaverHelper {
 public:
  BeaverHelper(const std::shared_ptr<OtAdapter>& ot_sender,
               const std::shared_ptr<OtAdapter>& ot_receiver) {
    ot_sender_ = ot_sender;
    ot_receiver_ = ot_receiver;
  }

  void MulPPSender(std::shared_ptr<Connection> conn,
                   absl::Span<internal::PTy> a, absl::Span<internal::PTy> c);

  void MulPPReceiver(std::shared_ptr<Connection> conn,
                     absl::Span<internal::PTy> b, absl::Span<internal::PTy> c);

  void BeaverTriple(std::shared_ptr<Connection> conn,
                    absl::Span<internal::PTy> a, absl::Span<internal::PTy> b,
                    absl::Span<internal::PTy> c);

 private:
  std::shared_ptr<OtAdapter> ot_sender_{nullptr};
  std::shared_ptr<OtAdapter> ot_receiver_{nullptr};
};

}  // namespace mcpsi::ot