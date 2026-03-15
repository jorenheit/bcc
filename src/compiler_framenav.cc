#include "compiler.ih"

void Compiler::pushFrame() {
  assert(_currentBlock != nullptr);
  assert(_currentFunction != nullptr);
  assert(_currentSeq != nullptr);


  pushPtr();
  
  // To push a frame, we need to move the pointer into the cell that marks the start of a fresh
  // frame, starting just beyond the current one. We also initialize the FrameID of this frame
  // with the current ID + 1.

  primitive::DInt currentFrameSize = [caller = _currentFunction->name](primitive::Context const &ctx){
    return ctx.getStackFrameSize(caller) * MacroCell::FieldCount;
  };
  
  moveToOrigin();
  switchField(MacroCell::FrameID);
  emit<primitive::CopyData>(MacroCell::FrameID,
			    currentFrameSize + MacroCell::FrameID,
			    currentFrameSize + MacroCell::Scratch0);
  emit<primitive::MovePointerRelative>(currentFrameSize);
  addConst(1);

  popPtr();
}

void Compiler::moveToPreviousFrame() {
  pushPtr();
  moveToOrigin();
  switchField(MacroCell::Flag0);
  setToValue(1);
  loopOpen(); {
    zeroCell();
    emit<primitive::MovePointerRelative>(-MacroCell::FieldCount);
    switchField(MacroCell::FrameID);
    notValue(MacroCell::Flag0);
    switchField(MacroCell::Flag0);
  } loopClose();
  popPtr();
}

void Compiler::moveToGlobalFrameWithPayload() {
  moveToGlobalFrame(true);
}

void Compiler::moveToGlobalFrame(bool payload) {
  pushPtr();
  
  moveToOrigin();
  markStartOfOriginFrame();
  switchField(MacroCell::Flag0);
  setToValue(1);
  loopOpen(); {
    zeroCell();
    if (payload) {
      switchField(MacroCell::Payload0);
      emit<primitive::MoveData>(-MacroCell::FieldCount);
    }    
    emit<primitive::MovePointerRelative>(-MacroCell::FieldCount);

    // Check if flag was hit by storing NOT(Flag1) in Flag0. If hit, flag0 becomes 0 and we exit the loop
    switchField(MacroCell::Flag1);
    emit<primitive::Not2>(MacroCell::Flag1, MacroCell::Flag0, MacroCell::Scratch0, MacroCell::Scratch1);
    switchField(MacroCell::Flag0);
  } loopClose();

  popPtr();
  
}

void Compiler::markStartOfOriginFrame() {
  moveToOrigin();
  switchField(MacroCell::Flag1);
  setToValue(1);
}

void Compiler::moveToOriginFrameWithPayload() {
  moveToOriginFrame(true);
}

void Compiler::moveToOriginFrame(bool payload) {
  pushPtr();
  switchField(MacroCell::Flag0);
  setToValue(1);
  loopOpen(); {
    zeroCell();
    // Move pointer and payload to the next cell
    if (payload) {
      switchField(MacroCell::Payload0);
      emit<primitive::MoveData>(MacroCell::FieldCount);
    }
    emit<primitive::MovePointerRelative>(MacroCell::FieldCount);

    // Check if flag was hit by storing NOT(Flag1) in Flag0. If hit, flag0 becomes 0 and we exit the loop
    switchField(MacroCell::Flag1);
    emit<primitive::Not2>(MacroCell::Flag1, MacroCell::Flag0, MacroCell::Scratch0, MacroCell::Scratch1);
    switchField(MacroCell::Flag0);
  } loopClose();

  switchField(MacroCell::Flag1);
  zeroCell();
  switchField(MacroCell::Value0);
  resetOrigin();
  popPtr();
}

void Compiler::popFrame() {
  assert(_currentBlock != nullptr);
  assert(_currentFunction != nullptr);
  assert(_currentSeq != nullptr);

  // Check if function is the entry-point of the program. If so, we need to set the run-cell to 0
  // If not, we do a dynamic move left until we hit the next frame marker.

  pushPtr();
  
  if (_currentFunction->name == _program.entryFunctionName) {
    moveTo(FrameLayout::RunState, MacroCell::Value0);
    zeroCell();
    moveToOrigin();
  }
  else {
    moveToOrigin();
    switchField(MacroCell::FrameID);
    zeroCell();
    moveToPreviousFrame();
    // Pointer should now be at the start of the previous frame
  }
  
  popPtr();
}


void Compiler::fetchReturnData() {
  assert(_currentSeq != nullptr);  
  assert(_currentFunction != nullptr);

  // Need to fetch:
  // 1. Run-state
  // 2. Return-values (not implemented)

  primitive::DInt const stackFrameSize = [caller = _currentFunction->name](primitive::Context const &ctx) -> int {
    return ctx.getStackFrameSize(caller) * MacroCell::FieldCount;
  };

  pushPtr();
  
  // Get run-state
  moveTo(FrameLayout::RunState);
  zeroCell();
  emit<primitive::MovePointerRelative>(stackFrameSize);
  emit<primitive::MoveData>(-stackFrameSize);
  emit<primitive::MovePointerRelative>(-stackFrameSize);

  popPtr();
}
