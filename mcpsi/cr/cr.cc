#include "mcpsi/cr/cr.h"

namespace mcpsi {

// register string
const std::string Correlation::id = std::string("Correlation");

BeaverTy Correlation::BeaverTriple(size_t num) {
  if (cache_ != nullptr && cache_->BeaverCacheSize() >= num) {
    return cache_->BeaverTriple(num);
  }
  SPDLOG_DEBUG("Miss match");
  std::vector<internal::ATy> a(num);
  std::vector<internal::ATy> b(num);
  std::vector<internal::ATy> c(num);
  BeaverTriple(absl::MakeSpan(a), absl::MakeSpan(b), absl::MakeSpan(c));
  return BeaverTy(std::move(a), std::move(b), std::move(c));
}

AuthTy Correlation::RandomSet(size_t num) {
  if (cache_ != nullptr && cache_->RandomSetSize() >= num) {
    return cache_->RandomSet(num);
  }
  SPDLOG_DEBUG("Miss match");
  std::vector<internal::ATy> ret(num);
  RandomSet(absl::MakeSpan(ret));
  return AuthTy(std::move(ret));
}

AuthTy Correlation::RandomGet(size_t num) {
  if (cache_ != nullptr && cache_->RandomGetSize() >= num) {
    return cache_->RandomGet(num);
  }
  SPDLOG_DEBUG("Miss match");
  std::vector<internal::ATy> ret(num);
  RandomGet(absl::MakeSpan(ret));
  return AuthTy(std::move(ret));
}

AuthTy Correlation::RandomAuth(size_t num) {
  if (cache_ != nullptr && cache_->RandomSetSize() >= num &&
      cache_->RandomGetSize() >= num) {
    AuthTy set = cache_->RandomSet(num);
    AuthTy get = cache_->RandomGet(num);
    std::vector<internal::ATy> out(num);
    internal::op::Add(
        absl::MakeConstSpan(
            reinterpret_cast<const internal::PTy*>(set.data.data()), 2 * num),
        absl::MakeConstSpan(
            reinterpret_cast<const internal::PTy*>(get.data.data()), 2 * num),
        absl::MakeSpan(reinterpret_cast<internal::PTy*>(out.data()), 2 * num));
    return AuthTy(std::move(out));
  }
  SPDLOG_DEBUG("Miss match");
  std::vector<internal::ATy> ret(num);
  RandomAuth(absl::MakeSpan(ret));
  return AuthTy(std::move(ret));
}

ShuffleSTy Correlation::ShuffleSet(size_t num, size_t repeat) {
  if (cache_ != nullptr && cache_->ShuffleSetCount(num, repeat)) {
    return cache_->ShuffleSet(num, repeat);
  }
  SPDLOG_DEBUG("Miss match");
  std::vector<internal::PTy> delta(num * repeat);
  std::vector<size_t> perm = GenPerm(num);
  ShuffleSet(absl::MakeSpan(perm), absl::MakeSpan(delta), repeat);
  return ShuffleSTy(std::move(delta), std::move(perm));
}

ShuffleGTy Correlation::ShuffleGet(size_t num, size_t repeat) {
  if (cache_ != nullptr && cache_->ShuffleGetCount(num, repeat)) {
    return cache_->ShuffleGet(num, repeat);
  }
  SPDLOG_DEBUG("Miss match");
  std::vector<internal::PTy> a(num * repeat);
  std::vector<internal::PTy> b(num * repeat);
  ShuffleGet(absl::MakeSpan(a), absl::MakeSpan(b), repeat);
  return ShuffleGTy(std::move(a), std::move(b));
}

// cache
void Correlation::force_cache(size_t beaver_num, size_t rand_set_num,
                              size_t rand_get_num,
                              const std::vector<uint64_t>& shuffle_set_shape,
                              const std::vector<uint64_t>& shuffle_get_shape) {
  cache_ = std::make_unique<CorrelationCache>();
  // beaver
  {
    cache_->beaver_cache = BeaverTy(beaver_num);
    auto& a = cache_->beaver_cache.a;
    auto& b = cache_->beaver_cache.b;
    auto& c = cache_->beaver_cache.c;
    BeaverTriple(absl::MakeSpan(a), absl::MakeSpan(b), absl::MakeSpan(c));
  }
  // random
  {
    cache_->random_set_cache = AuthTy(rand_set_num);
    cache_->random_get_cache = AuthTy(rand_get_num);
    auto& set_data = cache_->random_set_cache.data;
    auto& get_data = cache_->random_get_cache.data;
    if (ctx_->GetRank() == 0) {
      RandomSet(absl::MakeSpan(set_data));
      RandomGet(absl::MakeSpan(get_data));
    } else {
      RandomGet(absl::MakeSpan(get_data));
      RandomSet(absl::MakeSpan(set_data));
    }
  }
  // shuffle
  {
    auto& set_map = cache_->shuffle_set_cache;
    auto& get_map = cache_->shuffle_get_cache;
    if (ctx_->GetRank() == 0) {
      // set
      for (const auto index : shuffle_set_shape) {
        ShuffleSTy tmp(index >> 8, index & 0xFF);
        tmp.perm = GenPerm(index >> 8);
        ShuffleSet(absl::MakeSpan(tmp.perm), absl::MakeSpan(tmp.delta),
                   index & 0xFF);
        auto& vec = set_map[index];
        vec.emplace_back(tmp);
      }
      // get
      for (const auto index : shuffle_get_shape) {
        ShuffleGTy tmp(index >> 8, index & 0xFF);
        ShuffleGet(absl::MakeSpan(tmp.a), absl::MakeSpan(tmp.b), index & 0xFF);
        auto& vec = get_map[index];
        vec.emplace_back(tmp);
      }
    } else {
      // get
      for (const auto index : shuffle_get_shape) {
        ShuffleGTy tmp(index >> 8, index & 0xFF);
        ShuffleGet(absl::MakeSpan(tmp.a), absl::MakeSpan(tmp.b), index & 0xFF);
        auto& vec = get_map[index];
        vec.emplace_back(tmp);
      }
      // set
      for (const auto index : shuffle_set_shape) {
        ShuffleSTy tmp(index >> 8, index & 0xFF);
        tmp.perm = GenPerm(index >> 8);
        ShuffleSet(absl::MakeSpan(tmp.perm), absl::MakeSpan(tmp.delta),
                   index & 0xFF);
        auto& vec = set_map[index];
        vec.emplace_back(tmp);
      }
    }
  }
}

BeaverTy CorrelationCache::BeaverTriple(size_t num) {
  const size_t remain = BeaverCacheSize();
  YACL_ENFORCE(num <= remain);
  // copy
  std::vector<internal::ATy> a(beaver_cache.a.end() - num,
                               beaver_cache.a.end());
  std::vector<internal::ATy> b(beaver_cache.b.end() - num,
                               beaver_cache.b.end());
  std::vector<internal::ATy> c(beaver_cache.c.end() - num,
                               beaver_cache.c.end());
  // resize
  beaver_cache.a.resize(remain - num);
  beaver_cache.b.resize(remain - num);
  beaver_cache.c.resize(remain - num);

  return BeaverTy(std::move(a), std::move(b), std::move(c));
}

AuthTy CorrelationCache::RandomSet(size_t num) {
  const size_t remain = RandomSetSize();
  YACL_ENFORCE(num <= remain);
  // copy
  std::vector<internal::ATy> data(random_set_cache.data.end() - num,
                                  random_set_cache.data.end());
  // resize
  random_set_cache.data.resize(remain - num);

  return AuthTy(std::move(data));
}

AuthTy CorrelationCache::RandomGet(size_t num) {
  const size_t remain = RandomGetSize();
  YACL_ENFORCE(num <= remain);
  // copy
  std::vector<internal::ATy> data(random_get_cache.data.end() - num,
                                  random_get_cache.data.end());
  // resize
  random_get_cache.data.resize(remain - num);

  return AuthTy(std::move(data));
}

ShuffleSTy CorrelationCache::ShuffleSet(size_t num, size_t repeat) {
  YACL_ENFORCE(ShuffleSetCount(num, repeat) > 0);
  const uint64_t index = ((num << 8) | repeat);
  ShuffleSTy ret = std::move(shuffle_set_cache[index].back());
  shuffle_set_cache[index].pop_back();
  return ret;
}

ShuffleGTy CorrelationCache::ShuffleGet(size_t num, size_t repeat) {
  YACL_ENFORCE(ShuffleGetCount(num, repeat) > 0);
  const uint64_t index = ((num << 8) | repeat);
  ShuffleGTy ret = std::move(shuffle_get_cache[index].back());
  shuffle_get_cache[index].pop_back();
  return ret;
}

}  // namespace mcpsi
