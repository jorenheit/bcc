#pragma once

// class ValueField {
//   bool _known = false;
//   uint8_t _value = 0;

// public:
//   inline void set(uint8_t v = 0) {
//     _known = true;
//     _value = v;
//   }
  
//   inline void invalidate() { _known = false; }
//   inline operator uint8_t() const { return _value; }
//   inline bool known() const { return _known; }
  
//   inline ValueField &operator+=(uint8_t val) {
//     if (_known) _value += val;
//     return *this;
//   }

//   inline ValueField &operator-=(uint8_t val) {
//     if (_known) _value -= val;
//     return *this;
//   }

//   inline ValueField &operator*=(uint8_t val) {
//     if (_known) _value *= val;
//     return *this;
//   }

//   inline ValueField &operator/=(uint8_t val) {
//     if (_known) _value /= val;
//     return *this;
//   }
// };

struct Slot {

  enum Type {
    Local,
    Global,
    GlobalReference,
    TempUsed,
    TempFree
  };

  Type type;
  std::string name;
  int offset = 0;
  int size = 1;

  operator int() const { return offset; }
};
