// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
