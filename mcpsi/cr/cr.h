#pragma once
#include <memory>
#include <vector>

#include "mcpsi/context/context.h"
#include "mcpsi/context/state.h"
#include "mcpsi/ss/type.h"

namespace mcpsi {

struct BeaverTy {
  std::vector<internal::ATy> a;
  std::vector<internal::ATy> b;
  std::vector<internal::ATy> c;
  BeaverTy() { ; }
  BeaverTy(std::vector<internal::ATy>&& aa, std::vector<internal::ATy>&& bb,
           std::vector<internal::ATy>&& cc) {
    a = std::move(aa);
    b = std::move(bb);
    c = std::move(cc);
  }
  BeaverTy(uint32_t n) {
    a.resize(n);
    b.resize(n);
    c.resize(n);
  }
};

struct DyBeaverSetTy {
  std::vector<internal::ATy> a;
  std::vector<internal::ATy> b;
  std::vector<internal::ATy> c;
  DyBeaverSetTy() { ; }
  DyBeaverSetTy(std::vector<internal::ATy>&& aa,
                std::vector<internal::ATy>&& bb,
                std::vector<internal::ATy>&& cc) {
    a = std::move(aa);
    b = std::move(bb);
    c = std::move(cc);
  }
  DyBeaverSetTy(uint32_t n) {
    a.resize(n);
    b.resize(n);
    c.resize(n);
  }
};

struct DyBeaverGetTy {
  std::vector<internal::ATy> a;
  std::vector<internal::ATy> b;
  std::vector<internal::ATy> c;
  DyBeaverGetTy() { ; }
  DyBeaverGetTy(std::vector<internal::ATy>&& aa,
                std::vector<internal::ATy>&& bb,
                std::vector<internal::ATy>&& cc) {
    a = std::move(aa);
    b = std::move(bb);
    c = std::move(cc);
  }
  DyBeaverGetTy(uint32_t n) {
    a.resize(n);
    b.resize(n);
    c.resize(n);
  }
};

struct AuthTy {
  std::vector<internal::ATy> data;
  AuthTy() { ; }
  AuthTy(uint32_t n) { data.resize(n); }
  AuthTy(std::vector<internal::ATy>&& in) { data = std::move(in); }
};

struct ShuffleSTy {
  std::vector<internal::PTy> delta;
  std::vector<size_t> perm;
  ShuffleSTy() { ; }
  ShuffleSTy(uint32_t n, uint32_t repeat = 1) {
    delta.resize(n * repeat);
    perm.resize(n);
  }
  ShuffleSTy(std::vector<internal::PTy>&& _delta, std::vector<size_t>&& _perm) {
    delta = std::move(_delta);
    perm = std::move(_perm);
  }
};

struct ShuffleGTy {
  std::vector<internal::PTy> a;
  std::vector<internal::PTy> b;
  ShuffleGTy() { ; }
  ShuffleGTy(uint32_t n, uint32_t repeat = 1) {
    a.resize(n * repeat);
    b.resize(n * repeat);
  }
  ShuffleGTy(std::vector<internal::PTy>&& _a, std::vector<internal::PTy>&& _b) {
    a = std::move(_a);
    b = std::move(_b);
  }
};

struct CorrelationCache;

class Correlation : public State {
 protected:
  std::shared_ptr<Context> ctx_;
  internal::PTy key_;

 public:
  static const std::string id;

  Correlation(std::shared_ptr<Context> ctx) : ctx_(ctx) {}

  virtual ~Correlation() {}

  virtual internal::PTy GetKey() const { return key_; }

  virtual void SetKey(internal::PTy key) { key_ = key; }

  virtual void OneTimeSetup() = 0;

  // implementation
  virtual void BeaverTriple(absl::Span<internal::ATy> a,
                            absl::Span<internal::ATy> b,
                            absl::Span<internal::ATy> c) = 0;
  virtual void DyBeaverTripleSet(absl::Span<internal::ATy> a,
                                 absl::Span<internal::ATy> b,
                                 absl::Span<internal::ATy> c) = 0;
  virtual void DyBeaverTripleGet(absl::Span<internal::ATy> a,
                                 absl::Span<internal::ATy> b,
                                 absl::Span<internal::ATy> c) = 0;
  virtual void RandomSet(absl::Span<internal::ATy> out) = 0;
  virtual void RandomGet(absl::Span<internal::ATy> out) = 0;
  virtual void RandomAuth(absl::Span<internal::ATy> out) = 0;
  virtual void ShuffleSet(absl::Span<const size_t> perm,
                          absl::Span<internal::PTy> delta,
                          size_t repeat = 1) = 0;
  virtual void ShuffleGet(absl::Span<internal::PTy> a,
                          absl::Span<internal::PTy> b, size_t repeat = 1) = 0;

  // interface
  BeaverTy BeaverTriple(size_t num);
  DyBeaverSetTy DyBeaverTripleSet(size_t num);
  DyBeaverGetTy DyBeaverTripleGet(size_t num);
  AuthTy RandomSet(size_t num);
  AuthTy RandomGet(size_t num);
  AuthTy RandomAuth(size_t num);
  ShuffleSTy ShuffleSet(size_t num, size_t repeat = 1);
  ShuffleGTy ShuffleGet(size_t num, size_t repeat = 1);

  // ------------ cache -------------
 private:
  size_t b_num_{0};
  size_t b_s_num_{0};
  size_t b_g_num_{0};
  size_t r_s_num_{0};
  size_t r_g_num_{0};
  std::vector<uint64_t> s_s_shape_;
  std::vector<uint64_t> s_g_shape_;
  std::unique_ptr<CorrelationCache> cache_;

 public:
  // cache interface
  void BeaverTriple_cache(size_t num) { b_num_ += num; }
  void DyBeaverTripleSet_cache(size_t num) { b_s_num_ += num; }
  void DyBeaverTripleGet_cache(size_t num) { b_g_num_ += num; }
  void RandomSet_cache(size_t num) { r_s_num_ += num; }
  void RandomGet_cache(size_t num) { r_g_num_ += num; }
  void RandomAuth_cache(size_t num) {
    RandomGet_cache(num);
    RandomSet_cache(num);
  }
  void ShuffleSet_cache(size_t num, size_t repeat = 1) {
    s_s_shape_.emplace_back((num << 8) | repeat);
  }
  void ShuffleGet_cache(size_t num, size_t repeat = 1) {
    s_g_shape_.emplace_back((num << 8) | repeat);
  }

  // force cache
  void force_cache() {
    SPDLOG_INFO(
        "[P{}] FORCE CACHE!!! beaver num : {} , dy beaver get num : {} , dy "
        "beaver set num : {} , random set num : {} , random "
        "get num: {} , shuffle set num: {} , shuffle get num: {} ",
        ctx_->GetRank(), b_num_, b_s_num_, b_g_num_, r_s_num_, r_g_num_,
        s_s_shape_.size(), s_g_shape_.size());
    force_cache(b_num_, b_s_num_, b_g_num_, r_s_num_, r_g_num_, s_s_shape_,
                s_g_shape_);
  }

  void force_cache(size_t beaver_num, size_t dy_beaver_set_num,
                   size_t dy_beaver_get_num, size_t rand_set_num,
                   size_t rand_get_num,
                   const std::vector<uint64_t>& shuffle_set_shape = {},
                   const std::vector<uint64_t>& shuffle_get_shape = {});
};

struct CorrelationCache {
  BeaverTy beaver_cache;
  DyBeaverSetTy dy_beaver_set_cache;
  DyBeaverGetTy dy_beaver_get_cache;
  AuthTy random_set_cache;
  AuthTy random_get_cache;
  std::unordered_map<uint64_t, std::vector<ShuffleSTy>> shuffle_set_cache;
  std::unordered_map<uint64_t, std::vector<ShuffleGTy>> shuffle_get_cache;

  size_t BeaverCacheSize() { return beaver_cache.a.size(); }
  size_t DyBeaverSetCacheSize() { return dy_beaver_set_cache.a.size(); }
  size_t DyBeaverGetCacheSize() { return dy_beaver_get_cache.a.size(); }
  size_t RandomSetSize() { return random_set_cache.data.size(); }
  size_t RandomGetSize() { return random_get_cache.data.size(); }
  size_t ShuffleSetCount(size_t num, size_t repeat = 1) {
    uint64_t idx = ((num << 8) | repeat);
    return shuffle_set_cache.count(idx) ? shuffle_set_cache[idx].size() : 0;
  }
  size_t ShuffleGetCount(size_t num, size_t repeat = 1) {
    uint64_t idx = ((num << 8) | repeat);
    return shuffle_get_cache.count(idx) ? shuffle_get_cache[idx].size() : 0;
  }

  BeaverTy BeaverTriple(size_t num);
  DyBeaverSetTy DyBeaverTripleSet(size_t num);
  DyBeaverGetTy DyBeaverTripleGet(size_t num);
  AuthTy RandomSet(size_t num);
  AuthTy RandomGet(size_t num);
  ShuffleSTy ShuffleSet(size_t num, size_t repeat = 1);
  ShuffleGTy ShuffleGet(size_t num, size_t repeat = 1);
};

}  // namespace mcpsi
