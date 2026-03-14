#pragma once
#include <string>
#include <unordered_map>
#include <cassert>

// ============================================================
// MacroCell
// ============================================================


struct MacroCell {

  enum Field {
    FrameMarker,
    EnterFlag,
    Runtime0,
    Runtime1,
    Value, // TODO: move to field 0
    FieldCount
  };

  enum FrameMarkerValue {
    StackFrameMarkerValue = 1,
    GlobalVariableFrameMarkerValue = 2
  };
  
};

// ============================================================
// DataPointer
// ============================================================

class DataPointer {
  bool _static = true;
  int _staticOffset = 0;
  int _dynamicOffset = 0;
  MacroCell::Field _activeField = MacroCell::Value;

public:
  inline explicit DataPointer(int offset = 0, MacroCell::Field field = MacroCell::Value):
    _staticOffset(offset),
    _activeField(field)
  {}

  inline bool isStatic() const { return _static; }
  inline MacroCell::Field activeField() const { return _activeField; }
  inline void setField(MacroCell::Field field) { _activeField = field; }
    
  inline int staticOffset() const {
    assert(_static);
    return _staticOffset;
  }

  inline int dynamicOffset() const {
    assert(!_static);
    return _dynamicOffset;
  }

  inline void moveRelative(int logicalCells) {
    if (_static) _staticOffset += logicalCells;
    else _dynamicOffset += logicalCells;
  }

  inline void resetTo(int offset, MacroCell::Field field = MacroCell::Value) {
    _static = true;
    _staticOffset = offset;
    _dynamicOffset = 0;
    _activeField = field;
  }

  inline void beginDynamic() {
    assert(_static);
    _static = false;
    _dynamicOffset = 0;
  }

  inline void endDynamic() {
    assert(!_static);
    assert(_dynamicOffset == 0);
    _static = true;
  }
};
