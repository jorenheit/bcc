// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

template <typename ... Args>
Payload::Payload(Args ... args) {
  addPairs(args ...);
}

inline Payload::Width Payload::width(int index) const {
  assert(index < size());
  return units[index];
}

template <typename... Rest>
void Payload::addPairs(int count, Width width, Rest... rest) {
  assert(count > 0);
  for (int i = 0; i != count; ++i) {
    units.push_back(width);
  }
  addPairs(rest...);
}

template <size_t N>
template <typename ... Cells> requires (sizeof...(Cells) == N)
constexpr Temps<N>::Temps(Cells... cells):
  _cells{std::move(cells)...} {}

template <size_t N>
template <size_t I>
constexpr Cell const &Temps<N>::get() const {
  static_assert(I < N, "Temps index out of bounds");
  return _cells[I];
}

template <size_t N>
template <size_t ... Is>
constexpr auto Temps<N>::select() const {
  static_assert(sizeof ... (Is) > 0, "must be at least one index");
  return Temps<sizeof ... (Is)>{ _cells[Is] ... };
}

template <size_t N>
template <typename ... Args> requires (sizeof...(Args) > 0)
constexpr Temps<N> Temps<N>::select(Args ... args) {
  static_assert(sizeof...(Args) % 2 == 0,
                "makeTemps requires offset/field pairs");
  static_assert(sizeof...(Args) / 2 == N, "wrong number or args");
  auto tup = std::make_tuple(args...);

  return [&]<std::size_t... I>(std::index_sequence<I...>) {
    return Temps<N>{
      Cell{
        static_cast<int>(std::get<2 * I>(tup)),
        static_cast<MacroCell::Field>(std::get<2 * I + 1>(tup))
      }...
    };
  }(std::make_index_sequence<N>{});
}
