#pragma once

#include "yacl/base/dynamic_bitset.h"
#include "yacl/crypto/primitives/ot/base_ot.h"
#include "yacl/crypto/primitives/ot/kos_ote.h"
#include "yacl/crypto/primitives/ot/ot_store.h"
#include "yacl/crypto/utils/rand.h"

namespace mcpsi::vole {

namespace yc = yacl::crypto;
namespace yl = yacl::link;

class VoleAdapter {
 public:
  VoleAdapter() = default;
  virtual ~VoleAdapter() = default;

  virtual void rsend(absl::Span<internal::PTy> c) = 0;
  virtual void rrecv(absl::Span<internal::PTy> a,
                     absl::Span<internal::PTy> b) = 0;

  virtual void OneTimeSetup() = 0;
  virtual void OneTimeSetup(internal::PTy delta) = 0;

  internal::PTy delta_{0};
  virtual uint128_t GetDelta() const { return delta_; }
};

class WolverineVoleAdapter : public VoleAdapter {
 public:
  WolverineVoleAdapter(const std::shared_ptr<Connection>& conn,
                       std::shared_ptr<ot::OtAdapter> ot_ptr,
                       internal::PTy delta) {
    ot_ptr_ = ot_ptr;
    conn_ = conn;
    is_sender_ = ot_ptr_.IsSender();
    YACL_ENFORCE(is_sender_ == true);  // Vole Sender has delta
    delta_ = delta;
  }

  WolverineVoleAdapter(const std::shared_ptr<Connection>& conn,
                       std::shared_ptr<ot::OtAdapter> ot_ptr) {
    ot_ptr_ = ot_ptr;
    conn_ = conn;
    is_sender_ = ot_ptr_.IsSender();
    YACL_ENFORCE(is_sender_ == false);  // Vole Receiver
  }

  void rsend(absl::Span<internal::PTy> c) override;
  void rrecv(absl::Span<internal::PTy> a, absl::Span<internal::PTy> b) override;

  void OneTimeSetup() override;

  // Bootstrap would refresh Vole Buffer && Status
  void Bootstrap();
  // BoostrapInplace would generate voles in the span
  void BootstrapInplaceSend(absl::Span<internal::PTy> pre_c,
                            absl::Span<internal::PTy> c);

  void BootstrapInplaceRecv(absl::Span<internal::PTy> pre_a,
                            absl::Span<internal::PTy> pre_b,
                            absl::Span<internal::PTy> a,
                            absl::Span<internal::PTy> b);

 private:
  bool is_sender_{false};
  bool is_setup_{false};
  // Ot Adapter
  std::shared_ptr<Connection> conn_{nullptr};
  std::shared_ptr<ot::OtAdapter> ot_ptr_{nullptr};
  // Vole Buffer
  std::vector<internal::PTy> a_;
  std::vector<internal::PTy> b_;
  std::vector<internal::PTy> c_;
  // Vole Status
  uint64_t reserve_num_{0};
  uint64_t buff_used_num_{0};
  uint64_t buff_upper_bound_{0};
};

}  // namespace mcpsi::vole
