#pragma once
#include <memory>

namespace acus::proxy { 
  namespace Impl { 
    class Base; 
    struct SlotProxy; 
    using BasePtr = std::shared_ptr<Base>; 
  } 
  using SlotProxy = Impl::SlotProxy; 
}

namespace acus {
  using SlotProxy = proxy::Impl::SlotProxy;
}
