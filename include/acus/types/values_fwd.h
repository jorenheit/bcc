#pragma once

#include <memory>

namespace acus::values {
  namespace impl { struct Base; }
  using Literal = std::shared_ptr<impl::Base>;
}
