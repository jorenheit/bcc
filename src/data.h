#pragma once
#include <string>
#include <unordered_map>
#include <cassert>

// ============================================================
// MacroCell
// ============================================================


struct MacroCell {

  enum Field {
    Value0,
    Value1,
    FrameID,
    Flag,
    SeekMarker,
    Scratch0,
    Scratch1,
    Payload0,
    Payload1,
    
    FieldCount
  };
};

// ============================================================
// RuntimePointer
// ============================================================

struct RuntimePointer {
  enum Field {
    FrameDepth,
    Offset,
    Size
  };
};


// ============================================================
// Cell
// ============================================================

struct Cell {
  int offset = 0;
  MacroCell::Field field = MacroCell::Value0;
};

// ============================================================
// DataPointer
// ============================================================


class DataPointer {
  Cell _current;

public:
  Cell const &current() const { return _current; }
  MacroCell::Field field() const { return _current.field; }
  int offset() const { return _current.offset; }
  
  void moveRelative(int logicalCells) {
    _current.offset += logicalCells;
  }

  void set(MacroCell::Field field) {
    _current.field = field;
  }
  
  void set(int offset, MacroCell::Field field = MacroCell::Value0) {
    _current.offset = offset;
    _current.field = field;
  }

  void set(Cell cell) {
    set(cell.offset, cell.field);
  }
};


// ============================================================
// Temps
// ============================================================

template <size_t N>
struct Temps {
  std::array<Cell, N> _cells;

  constexpr Temps() = delete;

  template <typename ... Cells> requires (sizeof...(Cells) == N)
  constexpr Temps(Cells... cells):
    _cells{std::move(cells)...} {}

  template <size_t I>
  constexpr Cell const &get() const {
    static_assert(I < N, "Temps index out of bounds");
    return _cells[I];
  }

  template <size_t ...  Is>
  constexpr auto select() const {
    static_assert(sizeof ... (Is) > 0, "must be at least one index");
    return Temps<sizeof ... (Is)>{ _cells[Is] ... };
  }

  template <typename ... Args>
  static constexpr Temps pack(Args ... args) {
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
};
