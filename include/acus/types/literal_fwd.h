#pragma once

#include <memory>

namespace acus::literal {
  namespace impl { struct Base; }
  using Literal = std::shared_ptr<impl::Base>;
}
