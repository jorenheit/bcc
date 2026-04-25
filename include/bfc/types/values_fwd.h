#pragma once

#include <memory>

namespace values {
  namespace impl { struct Base; }
  using Literal = std::shared_ptr<impl::Base>;
}
