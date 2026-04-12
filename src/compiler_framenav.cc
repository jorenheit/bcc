#include "compiler.ih"

void Compiler::pushFrame() {
  assert(_currentBlock != nullptr);
  assert(_currentFunction != nullptr);
  assert(_currentSeq != nullptr);

  // To push a frame, we need to move the pointer into the cell that marks the start of a fresh
  // frame, starting just beyond the current one. We also increment the FrameMarker set its run-state to 1.

  primitive::DInt currentFrameSize = [caller = _currentFunction->name](primitive::Context const &ctx){
    return ctx.getStackFrameSize(caller) * MacroCell::FieldCount;
  };

  moveTo(0, MacroCell::FrameMarker);
  emit<primitive::CopyData>(getFieldIndex(0, MacroCell::FrameMarker),
			    currentFrameSize,
			    getFieldIndex(0, MacroCell::Scratch0));
  
  emit<primitive::MovePointerRelative>(currentFrameSize);
  inc();
  moveTo(FrameLayout::RunState, MacroCell::Value0);
  setToValue(1);
  moveToOrigin();
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
    switchField(MacroCell::FrameMarker);
    zeroCell();
    moveToPreviousFrame();
    // Pointer should now be at the start of the previous frame
  }
  popPtr();
}


void Compiler::seek(MacroCell::Field markerField, primitive::Direction dir, Payload const &payload, bool checkCurrent) {  
  int const stride = MacroCell::FieldCount * ((dir == primitive::Right) ? 1 : -1);

  if (not checkCurrent) {
    switchField(MacroCell::Flag);
    setToValue(1);
  }
  else {
    switchField(markerField);
    notConstructive(Cell{_dp.current().offset, MacroCell::Flag},
		    Temps<1>::pack(_dp.current().offset, MacroCell::Scratch0));
    switchField(MacroCell::Flag);
  }

  loopOpen(); {
    zeroCell();
    
    // Move pointer and payload to the next cell
    int const start = (dir == primitive::Right) ? payload.size() - 1 : 0;
    int const diff  = (dir == primitive::Right) ? -1 : 1;
    auto const cmp  = [&](int i)  { return (dir == primitive::Right) ? (i >= 0) : (i != payload.size()); };

    pushPtr();    
    moveRel(start);
    for (int i = start; cmp(i); i += diff) {
      switchField(MacroCell::Payload0);
      emit<primitive::MoveData>(stride);
      if (payload.width(i) == Payload::Width::Double) {
	switchField(MacroCell::Payload1);
	emit<primitive::MoveData>(stride);
      }
      moveRel(diff);
    }
    popPtr();
    emit<primitive::MovePointerRelative>(stride);
    
    // Check if flag was hit by storing NOT(SeekMarker) in Flag. If hit, flag0 becomes 0 and we exit the loop
    switchField(markerField);
    notConstructive(Cell{_dp.current().offset, MacroCell::Flag},
		    Temps<1>::pack(_dp.current().offset, MacroCell::Scratch0));
    switchField(MacroCell::Flag);
  } loopClose();
  switchField(static_cast<MacroCell::Field>(0));
}


void Compiler::setSeekMarker() {
  pushPtr();
  switchField(MacroCell::SeekMarker);
  setToValue(1);
  popPtr();
}

void Compiler::resetSeekMarker() {
  pushPtr();
  switchField(MacroCell::SeekMarker);
  zeroCell();
  popPtr();
}

void Compiler::moveToPreviousFrame(Payload const &payload) {
  moveToOrigin();
  seek(MacroCell::FrameMarker, primitive::Left, payload, false);
}


void Compiler::initializeArguments(std::string const &functionName, std::vector<values::RValue> const &args) {

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
  
  auto const constructInNextFrame = [&](auto&& self, int &offset, values::RValue const &arg) -> void {

    if (arg.hasSlot()) { // Already stored on tape -> copy to next frame
      Slot slot = arg.slot()->materialize(*this);

      // If it is a pointer that is being copied, increment its depth value
      if (types::isPointer(slot.type)) {
	Slot slotCopy = getTemp(slot.type);
	assignSlot(slotCopy, slot);
	moveTo(slotCopy + RuntimePointer::FrameDepth);
	inc();
	slot = slotCopy;
      }
      
      for (int i = 0; i != slot.type->size(); ++i) {
	int const varIndex0 = getFieldIndex(slot + i, MacroCell::Value0);
	primitive::DInt const paramIndex0 = currentFrameSize + paramStart + offset + MacroCell::Value0;
	primitive::DInt const scratchIndex = paramIndex0 + (MacroCell::Scratch0 - MacroCell::Value0);

	moveTo(slot + i, MacroCell::Value0);
	emit<primitive::CopyData>(varIndex0, paramIndex0, scratchIndex);
	moveTo(slot + i, MacroCell::Value1);
	if (slot.type->usesValue1()) {
	  int const varIndex1 = getFieldIndex(slot + i, MacroCell::Value1);
	  primitive::DInt const paramIndex1 = currentFrameSize + paramStart + offset + MacroCell::Value1;
	  emit<primitive::CopyData>(varIndex1, paramIndex1, scratchIndex);
	}
	else {
	  emit<primitive::ZeroCell>();
	}

	offset += MacroCell::FieldCount;
      }

    }
    else { // anonymous value -> construct in-place
      types::TypeHandle argType = arg.type();
      switch(argType->tag()) {
      case types::I8:
      case types::I16: {
	// Construct integer
	int const value = values::cast<types::IntegerType>(arg.value())->value();
	moveTo(0, MacroCell::Value0);
	primitive::DInt const diff = currentFrameSize + paramStart + offset;
	emit<primitive::MovePointerRelative>(diff);
	setToValue(value & 0xff);
	switchField(MacroCell::Value1);
	setToValue(argType->usesValue1() ? ((value >> 8) & 0xff) : 0);
	switchField(MacroCell::Value0);
	emit<primitive::MovePointerRelative>(-diff);
	offset += MacroCell::FieldCount;
	break;
      }
      case types::ARRAY:
      case types::STRING: {
	// recursive call for each element
	for (int i = 0; i != types::cast<types::ArrayLike>(argType)->length(); ++i)
	  self(self, offset, rValue(values::cast<types::ArrayLike>(arg.value())->element(i)));
	break;
      }
      default: assert(false && "passing this type as arg is not supported yet"); break;
      }
    }
  };

  pushPtr();
  int offset = 0;
  for (values::RValue const &arg: args) {
    constructInNextFrame(constructInNextFrame, offset, arg);
  }      
  popPtr();
}

void Compiler::fetchReturnData() {
  assert(_currentSeq != nullptr);  
  assert(_currentFunction != nullptr);

  primitive::DInt const stackFrameSize = [caller = _currentFunction->name](primitive::Context const &ctx) -> int {
    return ctx.getStackFrameSize(caller) * MacroCell::FieldCount;
  };

  pushPtr();
  
  // Get run-state
  moveTo(FrameLayout::RunState);
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
    emit<primitive::MovePointerRelative>(diff);
    emit<primitive::MoveData>(-diff);
    emit<primitive::MovePointerRelative>(-diff);

    if (returnSlot.type->usesValue1()) {
      moveTo(returnSlot + i, MacroCell::Value1);
      emit<primitive::MovePointerRelative>(diff);
      emit<primitive::MoveData>(-diff);
      emit<primitive::MovePointerRelative>(-diff);
    }
  }
  fetchReturnData(); // fetch the rest
  popPtr();
}

