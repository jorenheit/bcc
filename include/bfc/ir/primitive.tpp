#pragma once

namespace primitive {

  template <typename T, typename... Args>
  void Sequence::emplace(Args&&... args) {
    nodes.push_back(std::make_shared<T>(std::forward<Args>(args)...));
  }

} // namespace primitive
