#include "compiler.ih"

void Compiler::pushFrame() {
  assert(_currentBlock != nullptr);
  assert(_currentFunction != nullptr);
  assert(_currentSeq != nullptr);


  pushPtr();
  
  // To push a frame, we need to move the pointer into the cell that marks the start of a fresh
  // frame, starting just beyond the current one. We also initialize the FrameID of this frame
  // with the current ID + 1 and set its run-state to 1.

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

  moveTo(FrameLayout::RunState, MacroCell::Value0);
  setToValue(1);  
}

void Compiler::copyArgsToNextFrame(std::string const &functionName, std::vector<values::Value> const &args) {

  assert(_currentBlock != nullptr);
  assert(_currentFunction != nullptr);
  assert(_currentSeq != nullptr);

  // Type checking will be done later. Just copy all the arguments into the start of the
  // next stack frame. The callee will assume they are there in order.

  primitive::DInt const currentFrameSize = [caller = _currentFunction->name](primitive::Context const &ctx){
    return ctx.getStackFrameSize(caller) * MacroCell::FieldCount;
  };

  primitive::DInt const paramStart = [callee = functionName](primitive::Context const &ctx) {
    return ctx.getLocalBaseOffset(callee) * MacroCell::FieldCount;
  };
  
  auto const constructInNextFrame = [&](auto&& self, int &offset, auto&& arg) -> void {
    types::TypeHandle argType = arg->type(_ts);

    if (argType != nullptr) {
	
      if (types::isInteger(argType)) {
	int const value = arg->value();
	moveTo(0, MacroCell::Value0);
	primitive::DInt const diff = currentFrameSize + paramStart + offset;
	emit<primitive::MovePointerRelative>(diff);
	setToValue(value & 0xff);
	if (argType->usesValue1()) {
	  switchField(MacroCell::Value1);
	  setToValue((value >> 8) & 0xff);
	  switchField(MacroCell::Value0);
	}
	emit<primitive::MovePointerRelative>(-diff);
	offset += MacroCell::FieldCount;
      }
      else if (types::isArray(argType)) {
	// recursive call for each element
	for (int i = 0; i != argType->length(); ++i) {
	  auto const *elem = arg->element(i);
	  self(self, offset, elem);
	}
      }
      else {
	assert(false && "passing this type as arg is not supported yet");
      }
    }
    else {
      // This is a variable -> find its slot and copy the data into the next frame
      Slot const &varSlot = local(arg->varName());
      
      for (int i = 0; i != varSlot.type->size(); ++i) {
	int const varIndex0 = getFieldIndex(varSlot + i, MacroCell::Value0);
	primitive::DInt const paramIndex0 = currentFrameSize + paramStart + offset + MacroCell::Value0;
	primitive::DInt const scratchIndex = paramIndex0 + (MacroCell::Scratch0 - MacroCell::Value0);

	moveTo(varSlot + i, MacroCell::Value0);
	emit<primitive::CopyData>(varIndex0, paramIndex0, scratchIndex);
	if (varSlot.type->usesValue1()) {
	  int const varIndex1 = getFieldIndex(varSlot + i, MacroCell::Value1);
	  primitive::DInt const paramIndex1 = currentFrameSize + paramStart + offset + MacroCell::Value1;

	  moveTo(varSlot + i, MacroCell::Value1);
	  emit<primitive::CopyData>(varIndex1, paramIndex1, scratchIndex);
	}

	offset += MacroCell::FieldCount;
      }
    }
  };

  pushPtr();
  int offset = 0;
  for (auto const &arg: args) {
    constructInNextFrame(constructInNextFrame, offset, arg.get());
  }      
  popPtr();
}

void Compiler::moveToPreviousFrame() {
  pushPtr();
  moveToOrigin();
  switchField(MacroCell::Flag);
  setToValue(1);
  loopOpen(); {
    zeroCell();
    emit<primitive::MovePointerRelative>(-MacroCell::FieldCount);
    switchField(MacroCell::FrameID);
    notValue(MacroCell::Flag);
    switchField(MacroCell::Flag);
  } loopClose();
  popPtr();
}

void Compiler::moveToGlobalFrame(int payload) {
  assert(payload >= 0 && payload <= 2);
  pushPtr();
  
  moveToOrigin();
  markStartOfOriginFrame();
  switchField(MacroCell::Flag);
  setToValue(1);
  loopOpen(); {
    zeroCell();
    if (payload) {
      switchField(MacroCell::Payload0);
      emit<primitive::MoveData>(-MacroCell::FieldCount);
      if (payload == 2) {
	switchField(MacroCell::Payload1);
	emit<primitive::MoveData>(-MacroCell::FieldCount);
      }
    }    
    emit<primitive::MovePointerRelative>(-MacroCell::FieldCount);

    // Check if flag was hit by storing NOT(SeekMarker) in Flag. If hit, flag0 becomes 0 and we exit the loop
    switchField(MacroCell::SeekMarker);
    emit<primitive::Not2>(MacroCell::SeekMarker, MacroCell::Flag, MacroCell::Scratch0, MacroCell::Scratch1);
    switchField(MacroCell::Flag);
  } loopClose();

  popPtr();
}

void Compiler::markStartOfOriginFrame() {
  moveToOrigin();
  switchField(MacroCell::SeekMarker);
  setToValue(1);
}

void Compiler::moveToOriginFrame(int payload) {
  assert(payload >= 0 && payload <= 2);
  
  pushPtr();
  switchField(MacroCell::Flag);
  setToValue(1);
  loopOpen(); {
    zeroCell();
    // Move pointer and payload to the next cell
    if (payload) {
      switchField(MacroCell::Payload0);
      emit<primitive::MoveData>(MacroCell::FieldCount);
      if (payload == 2) {
	switchField(MacroCell::Payload1);
	emit<primitive::MoveData>(MacroCell::FieldCount);
      }
    }
    emit<primitive::MovePointerRelative>(MacroCell::FieldCount);

    // Check if flag was hit by storing NOT(SeekMarker) in Flag. If hit, flag0 becomes 0 and we exit the loop
    switchField(MacroCell::SeekMarker);
    emit<primitive::Not2>(MacroCell::SeekMarker, MacroCell::Flag, MacroCell::Scratch0, MacroCell::Scratch1);
    switchField(MacroCell::Flag);
  } loopClose();

  switchField(MacroCell::SeekMarker);
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

void Compiler::fetchReturnData(Slot const &returnSlot) {

  primitive::DInt const stackFrameSize = [caller = _currentFunction->name](primitive::Context const &ctx) -> int {
    return ctx.getStackFrameSize(caller) * MacroCell::FieldCount;
  };

  pushPtr();
  for (int i = 0; i != returnSlot.type->size(); ++i) {
    primitive::DInt const diff = stackFrameSize - (returnSlot - FrameLayout::ReturnValueStart) * MacroCell::FieldCount;
    
    moveTo(returnSlot + i, MacroCell::Value0);
    zeroCell();
    emit<primitive::MovePointerRelative>(diff);
    emit<primitive::MoveData>(-diff);
    emit<primitive::MovePointerRelative>(-diff);

    if (returnSlot.type->usesValue1()) {
      moveTo(returnSlot + i, MacroCell::Value1);
      zeroCell();
      emit<primitive::MovePointerRelative>(diff);
      emit<primitive::MoveData>(-diff);
      emit<primitive::MovePointerRelative>(-diff);
    }
  }
  fetchReturnData(); // fetch the rest
  popPtr();
}
