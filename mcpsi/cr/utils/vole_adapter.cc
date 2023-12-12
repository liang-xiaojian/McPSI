#include "mcpsi/cr/utils/vole_adapter.h"

namespace mcpsi::vole {

void WolverineVoleAdapter::OneTimeSetup() {
  auto setup_param = VoleParam(LpnParam::GetPreDefault());
  auto& setup_mp_param = setup_param.mp_param_;
  auto& setup_lpn_param = setup_param.lpn_param_;

  auto ot_num = setup_mp_param.require_ot_num_;
  //   SPDLOG_INFO("OneTimeSetup isSender {}", is_sender_);
  if (is_sender_) {
    std::vector<internal::PTy> pre_c(setup_lpn_param.k_, 0);
    ot::OtHelper(ot_ptr_, nullptr)
        .BaseVoleSend(conn_, delta_, absl::MakeSpan(pre_c));

    std::vector<uint128_t> send_msgs(ot_num);
    ot_ptr_->send_rcot(absl::MakeSpan(send_msgs));
    auto send_store =
        yc::MakeCompactOtSendStore(std::move(send_msgs), ot_ptr_->GetDelta());
    // SPDLOG_INFO("Wolverine Send");
    WolverineVoleSend(conn_, send_store, setup_param, absl::MakeSpan(pre_c),
                      absl::MakeSpan(c_));

  } else {
    std::vector<internal::PTy> pre_a(setup_lpn_param.k_, 0);
    std::vector<internal::PTy> pre_b(setup_lpn_param.k_, 0);
    ot::OtHelper(nullptr, ot_ptr_)
        .BaseVoleRecv(conn_, absl::MakeSpan(pre_a), absl::MakeSpan(pre_b));

    std::vector<uint128_t> recv_msgs(ot_num);
    yacl::dynamic_bitset<uint128_t> choices(ot_num);
    ot_ptr_->recv_rcot(absl::MakeSpan(recv_msgs), choices);
    auto recv_store = yc::MakeOtRecvStore(choices, std::move(recv_msgs));
    // SPDLOG_INFO("Wolverine Recv");
    setup_param.mp_param_.GenIndexes();
    WolverineVoleRecv(conn_, recv_store, setup_param, absl::MakeSpan(pre_a),
                      absl::MakeSpan(pre_b), absl::MakeSpan(a_),
                      absl::MakeSpan(b_));
  }

  reserve_num_ = vole_param_.lpn_param_.k_;
  buff_used_num_ = reserve_num_;
  buff_upper_bound_ = setup_lpn_param.n_;
  is_setup_ = true;
  //   SPDLOG_INFO("OneTimeSetup Done");
}

void WolverineVoleAdapter::rsend(absl::Span<internal::PTy> c) {
  YACL_ENFORCE(is_sender_ == true);

  //   SPDLOG_INFO("Call RSEND");
  if (is_setup_ == false) {
    OneTimeSetup();
  }

  auto& lpn_param = vole_param_.lpn_param_;

  uint64_t data_offset = 0;
  uint64_t require_num = c.size();
  uint64_t remain_num = buff_upper_bound_ - buff_used_num_;

  {
    uint32_t bootstrap_inplace_counter = 0;
    YACL_ENFORCE(reserve_num_ == lpn_param.k_);
    absl::Span<internal::PTy> c_span = absl::MakeSpan(c_.data(), reserve_num_);
    while (require_num > lpn_param.n_) {
      // avoid memory copy
      BootstrapInplaceSend(c_span, c.subspan(data_offset, lpn_param.n_));

      data_offset += (lpn_param.n_ - reserve_num_);
      require_num -= (lpn_param.n_ - reserve_num_);
      ++bootstrap_inplace_counter;
      // Next Round
      c_span = c.subspan(data_offset, reserve_num_);
    }
    if (bootstrap_inplace_counter != 0) {
      memcpy(c_.data(), c_span.data(), reserve_num_ * sizeof(internal::PTy));
    }
  }

  uint64_t vole_num = std::min(remain_num, require_num);

  memcpy(c.data() + data_offset, c_.data() + buff_used_num_,
         vole_num * sizeof(internal::PTy));

  buff_used_num_ += vole_num;
  // add state
  data_offset += vole_num;

  // In the case of running out of ot_buff_
  if (vole_num < require_num) {
    require_num -= vole_num;
    Bootstrap();

    // Worst Case
    // Require_num is greater then "buff_upper_bound_ - reserve_num_"
    // which means that an extra "Bootstrap" is needed.
    if (require_num > (buff_upper_bound_ - reserve_num_)) {
      SPDLOG_INFO("[VoleAdapter] Worst Case Occured!!! current require_num {}",
                  require_num);
      // Bootstrap would reset buff_used_num_
      memcpy(c.data() + data_offset, c_.data() + buff_used_num_,
             (buff_upper_bound_ - reserve_num_) * sizeof(internal::PTy));
      require_num -= (buff_upper_bound_ - reserve_num_);
      data_offset += (buff_upper_bound_ - reserve_num_);
      // Bootstrap would reset buff_used_num_
      Bootstrap();
    }
    memcpy(c.data() + data_offset, c_.data() + buff_used_num_,
           require_num * sizeof(internal::PTy));
    buff_used_num_ += require_num;
  }
}

void WolverineVoleAdapter::rrecv(absl::Span<internal::PTy> a,
                                 absl::Span<internal::PTy> b) {
  YACL_ENFORCE(is_sender_ == false);
  YACL_ENFORCE(a.size() == b.size());

  //   SPDLOG_INFO("Call RRECV");
  if (is_setup_ == false) {
    OneTimeSetup();
  }

  auto& lpn_param = vole_param_.lpn_param_;
  uint64_t data_offset = 0;
  uint64_t require_num = a.size();
  uint64_t remain_num = buff_upper_bound_ - buff_used_num_;

  {
    uint32_t bootstrap_inplace_counter = 0;
    YACL_ENFORCE(reserve_num_ == lpn_param.k_);
    absl::Span<internal::PTy> a_span = absl::MakeSpan(a_.data(), reserve_num_);
    absl::Span<internal::PTy> b_span = absl::MakeSpan(b_.data(), reserve_num_);
    while (require_num > lpn_param.n_) {
      // avoid memory copy
      BootstrapInplaceRecv(a_span, b_span, a.subspan(data_offset, lpn_param.n_),
                           b.subspan(data_offset, lpn_param.n_));

      data_offset += (lpn_param.n_ - reserve_num_);
      require_num -= (lpn_param.n_ - reserve_num_);
      ++bootstrap_inplace_counter;
      // Next Round
      a_span = a.subspan(data_offset, reserve_num_);
      b_span = b.subspan(data_offset, reserve_num_);
    }
    if (bootstrap_inplace_counter != 0) {
      memcpy(a_.data(), a_span.data(), reserve_num_ * sizeof(internal::PTy));
      memcpy(b_.data(), b_span.data(), reserve_num_ * sizeof(internal::PTy));
    }
  }

  uint64_t vole_num = std::min(remain_num, require_num);

  memcpy(a.data() + data_offset, a_.data() + buff_used_num_,
         vole_num * sizeof(internal::PTy));
  memcpy(b.data() + data_offset, b_.data() + buff_used_num_,
         vole_num * sizeof(internal::PTy));

  buff_used_num_ += vole_num;
  // add state
  data_offset += vole_num;

  // In the case of running out of ot_buff_
  if (vole_num < require_num) {
    require_num -= vole_num;
    Bootstrap();

    // Worst Case
    // Require_num is greater then "buff_upper_bound_ - reserve_num_"
    // which means that an extra "Bootstrap" is needed.
    if (require_num > (buff_upper_bound_ - reserve_num_)) {
      SPDLOG_INFO("[VoleAdapter] Worst Case Occured!!! current require_num {}",
                  require_num);
      // Bootstrap would reset buff_used_num_
      memcpy(a.data() + data_offset, a_.data() + buff_used_num_,
             (buff_upper_bound_ - reserve_num_) * sizeof(internal::PTy));
      memcpy(b.data() + data_offset, b_.data() + buff_used_num_,
             (buff_upper_bound_ - reserve_num_) * sizeof(internal::PTy));
      require_num -= (buff_upper_bound_ - reserve_num_);
      data_offset += (buff_upper_bound_ - reserve_num_);
      // Bootstrap would reset buff_used_num_
      Bootstrap();
    }
    memcpy(a.data() + data_offset, a_.data() + buff_used_num_,
           require_num * sizeof(internal::PTy));
    memcpy(b.data() + data_offset, b_.data() + buff_used_num_,
           require_num * sizeof(internal::PTy));
    buff_used_num_ += require_num;
  }
}

void WolverineVoleAdapter::Bootstrap() {
  auto& lpn_param = vole_param_.lpn_param_;

  YACL_ENFORCE(lpn_param.k_ == reserve_num_);
  auto ot_num = vole_param_.mp_param_.require_ot_num_;

  if (is_sender_) {
    // prepare OT
    std::vector<uint128_t> send_msgs(ot_num);
    ot_ptr_->send_rcot(absl::MakeSpan(send_msgs));
    auto send_store =
        yc::MakeCompactOtSendStore(std::move(send_msgs), ot_ptr_->GetDelta());
    // Copy
    std::vector<internal::PTy> tmp_c(c_.begin(), c_.begin() + reserve_num_);

    WolverineVoleSend(conn_, send_store, vole_param_, absl::MakeSpan(tmp_c),
                      absl::MakeSpan(c_));
  } else {
    // prepare OT
    std::vector<uint128_t> recv_msgs(ot_num);
    yacl::dynamic_bitset<uint128_t> choices(ot_num);
    ot_ptr_->recv_rcot(absl::MakeSpan(recv_msgs), choices);
    auto recv_store = yc::MakeOtRecvStore(choices, std::move(recv_msgs));
    // Copy
    std::vector<internal::PTy> tmp_a(a_.begin(), a_.begin() + reserve_num_);
    std::vector<internal::PTy> tmp_b(b_.begin(), b_.begin() + reserve_num_);
    // Wolverine
    vole_param_.mp_param_.GenIndexes();
    WolverineVoleRecv(conn_, recv_store, vole_param_, absl::MakeSpan(tmp_a),
                      absl::MakeSpan(tmp_b), absl::MakeSpan(a_),
                      absl::MakeSpan(b_));
  }
  reserve_num_ = lpn_param.k_;
  buff_used_num_ = reserve_num_;
  buff_upper_bound_ = lpn_param.n_;
}

void WolverineVoleAdapter::BootstrapInplaceSend(absl::Span<internal::PTy> pre_c,
                                                absl::Span<internal::PTy> c) {
  auto lpn_param = vole_param_.lpn_param_;

  YACL_ENFORCE(is_sender_ == true);
  YACL_ENFORCE(pre_c.size() >= lpn_param.k_);
  auto ot_num = vole_param_.mp_param_.require_ot_num_;
  // prepare OT
  std::vector<uint128_t> send_msgs(ot_num);
  ot_ptr_->send_rcot(absl::MakeSpan(send_msgs));
  auto send_store =
      yc::MakeCompactOtSendStore(std::move(send_msgs), ot_ptr_->GetDelta());
  // Copy
  std::vector<internal::PTy> tmp_c(pre_c.begin(), pre_c.begin() + lpn_param.k_);

  WolverineVoleSend(conn_, send_store, vole_param_, absl::MakeSpan(tmp_c),
                    absl::MakeSpan(c));
}

void WolverineVoleAdapter::BootstrapInplaceRecv(absl::Span<internal::PTy> pre_a,
                                                absl::Span<internal::PTy> pre_b,
                                                absl::Span<internal::PTy> a,
                                                absl::Span<internal::PTy> b) {
  auto lpn_param = vole_param_.lpn_param_;

  YACL_ENFORCE(is_sender_ == true);
  YACL_ENFORCE(pre_a.size() >= lpn_param.k_);
  YACL_ENFORCE(pre_b.size() >= lpn_param.k_);
  auto ot_num = vole_param_.mp_param_.require_ot_num_;
  // prepare OT
  std::vector<uint128_t> recv_msgs(ot_num);
  yacl::dynamic_bitset<uint128_t> choices(ot_num);
  ot_ptr_->recv_rcot(absl::MakeSpan(recv_msgs), choices);
  auto recv_store = yc::MakeOtRecvStore(choices, std::move(recv_msgs));
  // Copy
  std::vector<internal::PTy> tmp_a(pre_a.begin(), pre_a.begin() + lpn_param.k_);
  std::vector<internal::PTy> tmp_b(pre_b.begin(), pre_b.begin() + lpn_param.k_);
  // Wolverine
  vole_param_.mp_param_.GenIndexes();
  WolverineVoleRecv(conn_, recv_store, vole_param_, absl::MakeSpan(tmp_a),
                    absl::MakeSpan(tmp_b), absl::MakeSpan(a),
                    absl::MakeSpan(b));
}

}  // namespace mcpsi::vole
