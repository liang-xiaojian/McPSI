#include "mcpsi/cr/utils/vole_adapter.h"

#include "mcpsi/cr/utils/vole.h"

namespace mcpsi::vole {

void WolverineVoleAdapter::OneTimeSetup() {
  auto setup_param = LpnParam::GetPreDefault();
  auto lpn_parm = LpnParam::GetDefault();
  auto ot_num = setup_param.mp_param_.require_ot_num_;

  if (is_sender_) {
    std::vector<internal::PTy> pre_c(setup_param.k_);
    OtHelper(ot_ptr_, nullptr)
        .BaseVoleSend(conn_, absl::MakeSpan(pre_c), delta_);

    std::vector<uint128_t> send_msgs(ot_num);
    ot_ptr_->send_rcot(absl::MakeSpan(send_msgs));
    auto send_store =
        yc::MakeCompactOtSendStore(std::move(send_msgs), ot_ptr_->GetDelta());

    c_.resize(lpn_parm.n_);
    WolverineVoleSend(conn_, send_store, setup_param, absl::MakeSpan(pre_c),
                      absl::MakeSpan(c_));

  } else {
    std::vector<internal::PTy> pre_a(setup_param.k_);
    std::vector<internal::PTy> pre_b(setup_param.k_);
    OtHelper(nullptr, ot_ptr_)
        .BaseVoleRecv(conn_, absl::MakeSpan(pre_a), absl::MakeSpan(pre_b));

    std::vector<uint128_t> recv_msgs(ot_num);
    yacl::dynamic_bitset<uint128_t> choices(ot_num);
    ot_ptr_->recv_rcot(absl::MakeSpan(recv_msgs), choices);
    auto recv_store = yc::MakeOtRecvStore(std::move(recv_msgs), choices);

    setup_param.mp_param_.GenIndexes();
    a_.resize(lpn_parm.n_);
    b_.resize(lpn_parm.n_);
    WolverineVoleRecv(conn_, recv_store, setup_param, absl::MakeSpan(pre_a),
                      absl::MakeSpan(pre_b), absl::MakeSpan(a_),
                      absl::MakeSpan(b_));
  }

  reserve_num_ = lpn_parm.mp_param_.n_;
  buff_used_num_ = reserve_num_;
  buff_upper_bound_ = setup_param.mp_param_.n_;
}

}  // namespace mcpsi::vole
