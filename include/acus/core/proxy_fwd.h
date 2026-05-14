// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
