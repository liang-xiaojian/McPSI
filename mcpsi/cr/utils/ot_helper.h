#pragma once

#include "mcpsi/context/state.h"
#include "mcpsi/cr/utils/ot_adapter.h"
#include "mcpsi/ss/type.h"

namespace mcpsi::ot {

class OtHelper {
 public:
  OtHelper(const std::shared_ptr<OtAdapter>& ot_sender,
           const std::shared_ptr<OtAdapter>& ot_receiver) {
    ot_sender_ = ot_sender;
    ot_receiver_ = ot_receiver;
  }

  void MulPPSend(std::shared_ptr<Connection> conn, absl::Span<internal::PTy> b,
                 absl::Span<internal::PTy> c);

  void MulPPRecv(std::shared_ptr<Connection> conn, absl::Span<internal::PTy> a,
                 absl::Span<internal::PTy> c);

  void BeaverTriple(std::shared_ptr<Connection> conn,
                    absl::Span<internal::PTy> a, absl::Span<internal::PTy> b,
                    absl::Span<internal::PTy> c);

  // a * b = c && A * b = C
  void MulPPExtendSend(std::shared_ptr<Connection> conn,
                       absl::Span<internal::PTy> b, absl::Span<internal::PTy> c,
                       absl::Span<internal::PTy> C);

  // a * b = c && A * b = C
  void MulPPExtendRecv(std::shared_ptr<Connection> conn,
                       absl::Span<internal::PTy> a, absl::Span<internal::PTy> c,
                       absl::Span<internal::PTy> A,
                       absl::Span<internal::PTy> C);

  // a * b = c && A * b = C
  void BeaverTripleExtend(std::shared_ptr<Connection> conn,
                          absl::Span<internal::PTy> a,
                          absl::Span<internal::PTy> b,
                          absl::Span<internal::PTy> c,
                          absl::Span<internal::PTy> A,
                          absl::Span<internal::PTy> C);

  void BaseVoleSend(std::shared_ptr<Connection> conn, internal::PTy delta,
                    absl::Span<internal::PTy> c);

  void BaseVoleRecv(std::shared_ptr<Connection> conn,
                    absl::Span<internal::PTy> a, absl::Span<internal::PTy> b);

  void ShuffleSend(std::shared_ptr<Connection> conn,
                   absl::Span<const size_t> perm,
                   absl::Span<internal::PTy> delta);

  void ShuffleRecv(std::shared_ptr<Connection> conn,
                   absl::Span<internal::PTy> a, absl::Span<internal::PTy> b);

 private:
  std::shared_ptr<OtAdapter> ot_sender_{nullptr};
  std::shared_ptr<OtAdapter> ot_receiver_{nullptr};
};

}  // namespace mcpsi::ot