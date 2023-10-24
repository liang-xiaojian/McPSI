#pragma once

#include <__nullptr>
#include <cstddef>
#include <map>
#include <memory>
#include <string>

#include "yacl/crypto/tools/prg.h"

namespace test {
class State {
 public:
  virtual ~State() = default;
};

class Prg : public State {
 public:
  static const std::string id;
  std::shared_ptr<yacl::crypto::Prg<uint8_t>> prg_{nullptr};

  template <typename... Args>
  Prg(Args&&... args) {
    prg_ = std::make_shared<yacl::crypto::Prg<uint8_t>>(
        std::forward<Args>(args)...);
  }
};

class StateList final {
 private:
  std::map<std::string, std::shared_ptr<State>> states_;

 public:
  StateList() : states_() {}

  template <typename StateTy>
  void InsertState(std::shared_ptr<StateTy> state) {
    states_.insert(StateTy::id, state);
  }

  template <typename StateTy, typename... Args>
  void InsertState(Args&&... args) {
    states_.emplace(StateTy::id,
                    std::make_shared<StateTy>(std::forward<Args>(args)...));
  }

  template <typename StateTy>
  std::shared_ptr<StateTy> GetState() {
    auto iter = states_.find(StateTy::id);
    YACL_ENFORCE(iter != states_.end());
    return std::dynamic_pointer_cast<StateTy>(iter->second);
  }
};

}  // namespace test
