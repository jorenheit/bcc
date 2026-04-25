#pragma once
#include <memory>

namespace proxy { 
  namespace Impl { 
    class Base; 
    struct SlotProxy; 
    using BasePtr = std::shared_ptr<Base>; 
  } 
  using SlotProxy = Impl::SlotProxy; 
}
using SlotProxy = proxy::Impl::SlotProxy;
