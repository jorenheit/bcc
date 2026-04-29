#pragma once

#include <memory>

namespace acus::literal {
  namespace impl {
    struct Base;
    using Literal = std::shared_ptr<Base>;
    
    template <typename V> requires std::derived_from<V, Base>
    auto cast(Literal const &v);
    
  }
}
