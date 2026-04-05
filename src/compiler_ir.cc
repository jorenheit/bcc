#include "compiler.ih"

void Compiler::setEntryPoint(std::string functionName) {
  _program.entryFunctionName = std::move(functionName);
}

void Compiler::begin() {
  error_if(_program.entryFunctionName.empty(),
	   "No entry point set before calling 'begin'; call 'setEntryPoint' first.");
  error_if(_state.begun, "called 'begin' before ending previous program.");

  _state.begun = true;
}

void Compiler::end() {
  error_if(not _state.begun, "called 'end' before 'begin'.");
  error_if(_currentFunction != nullptr, "called 'end' before ending function '", _currentFunction->name, "'."); 
  error_if(_currentBlock != nullptr, "called 'end' before ending block '", _currentBlock->name, "'."); 
  error_if(_currentScope != nullptr, "called 'end' before ending scope."); 
  
  functionCallTypeChecks();
  
  // Done compiling the program. Generate the metablocks, bootstrap and hatstrap sequences.
  constructMetaBlocks();

  
  // To bootstrap the system, we need to do the following:
  // 1. Mark cell 0 using the FrameID field to indicate that this is where
  //    the global data frame starts. Leave the pointer in the value-field.
  // 2. Move the pointer to the first stackframe and reset the origin.
  // 3. Initialize the first stack-frame, where we set the TargetBlock to the
  //    index of the entry-function and the run-state to 1.
  // 4. Open the main-loop and leave the pointer in cell 0 of the frame.

  setTargetSequence(&_program.bootstrap);

  resetOrigin();
  _dp.set(static_cast<MacroCell::Field>(0));
  switchField(MacroCell::SeekMarker);
  addConst(1);
  switchField(MacroCell::Value0);
  moveTo(1 + _program.globalVariableFrameSize()); 

  resetOrigin();
  switchField(MacroCell::FrameID);
  addConst(FrameLayout::FirstStackFrameID);

  setNextBlock(_program.entryFunctionName, "");
  moveTo(FrameLayout::RunState, MacroCell::Value0);
  setToValue(1);
  loopOpen("main loop");
  moveToOrigin();
  
  // Also generate the hatstrap code. All this needs to do is move the pointer to
  // the run-cell and close the loop.

  setTargetSequence(&_program.hatstrap);
  moveTo(FrameLayout::RunState, MacroCell::Value0);
  loopClose("main loop");
}


void Compiler::beginFunction(std::string const &name, FunctionSignature const &sig) {
  error_if(not _state.begun, "called 'beginFunction(", name, ")' outside of a program-block; call 'begin' first.");  
  error_if(_currentFunction != nullptr, "called 'beginFunction(", name, ")' while in existing function-block; call 'endFunction' first.");
  error_if(_currentScope != nullptr,    "called 'beginFunction(", name, ")' while in a scope-block; call 'endScope' first.");
  error_if(_currentBlock != nullptr,    "called 'beginFunction(", name, ")' while in a code-block; call 'endBlock' first.");
  _state.allowGlobalDefinitions = false;
  
  _currentFunction = &_program.createFunction(name, sig);  
  for (FunctionParam const &p: sig.params) {
    declareLocal(p.name, p.type);
  }

}

void Compiler::endFunction() {  
  error_if(_currentFunction == nullptr, "called 'endFunction' outside of a function-block.");
  error_if(_currentBlock != nullptr,    "called 'endFunction' (", _currentFunction->name, ") while inside a code-block (", _currentBlock->name, "); ""call 'endBlock' first.");
  error_if(_currentScope != nullptr,    "called 'endFunction' (", _currentFunction->name, ") while inside a scope-block; call 'endScope' first.");
  
  _currentFunction = nullptr;
}

void Compiler::beginScope() {
  error_if(not _state.begun, "called 'beginScope' outside of a program-block; call 'begin' first.");    
  error_if(_currentFunction == nullptr, "called 'beginScope' outside of a function-block; scopes are only defined within functions.");
  error_if(_currentBlock != nullptr,    "called 'beginScope' inside of a code-block (", _currentBlock->name, "); scopes can only enclose code-blocks.");
  
  _currentScope = &_currentFunction->createScope(_currentScope);
}

void Compiler::endScope() {
  error_if(_currentFunction == nullptr, "called 'endScope' outside of a function-block; scopes care only defined within functions.");
  error_if(_currentScope == nullptr,    "called 'endScope' before calling 'beginScope'.");
  error_if(_currentBlock != nullptr,    "called 'endScope' inside of a code-block (", _currentBlock->name, "); scopes can only enclose code-blocks.");
  
  freeScope(_currentScope);
  _currentScope = _currentScope->parent;
}


void Compiler::beginBlock(std::string name) {
  error_if(not _state.begun, "called 'beginBlock(", name, ")' outside of a program-block; call 'begin' first.");    
  error_if(_currentFunction == nullptr, "called 'beginBlock(", name, ")' outside of a function-block; blocks are only defined within functions.");
  error_if(_currentBlock != nullptr,    "called 'beginBlock(", name, ")' before ending the current block (", _currentBlock->name, ").");

  auto globalIdx = _program.nextGlobalBlockIndex();
  Function::Block &block = _currentFunction->createBlock(std::move(name), globalIdx);
  _program.registerBlock(block);

  if (_currentFunction->blocks.size() == 1) {
    _currentFunction->entryBlockIndex = globalIdx;
  }

  _currentBlock = &block;
  resetOrigin();
  _nextBlockIsSet = false;

  setTargetSequence(&block.code);
  blockOpen();
}

void Compiler::endBlock() {
  error_if(_currentBlock == nullptr, "called 'endBlock' before calling 'beginBlock'.");
  assert(_nextBlockIsSet);

  blockClose();
  freeTemps();
  _currentBlock = nullptr;
}

void Compiler::setNextBlock(int index) {
  assert(_currentSeq != nullptr);

  pushPtr();
  moveTo(FrameLayout::TargetBlock, MacroCell::Value0);
  setToValue16(index, Cell{FrameLayout::TargetBlock, MacroCell::Value1});
  popPtr();
  _nextBlockIsSet = true;
}
    
void Compiler::setNextBlock(std::string f, std::string b) {
  assert(_currentSeq != nullptr);

  // It is possible that the function or block name has not been
  // defined yet. So we need to check for this first.
  if (_program.isFunctionDefined(f)) {
    Function const &func = _program.function(f);
    if (func.isBlockDefined(b)) {
      // both defined -> use global block index
      return setNextBlock(func.block(b).globalBlockIndex);
    }
  }

  pushPtr();

  // Could not determine block index yet -> postpone until actual code generation
  moveTo(FrameLayout::TargetBlock, MacroCell::Value0);
  zeroCell();
  emit<primitive::ChangeBy>([f, b](primitive::Context const &ctx) -> int {
    return ctx.getBlockIndex(f, b) & 0xff;
  });

  moveTo(FrameLayout::TargetBlock, MacroCell::Value1);
  zeroCell();
  emit<primitive::ChangeBy>([f, b](primitive::Context const &ctx) -> int {
    return (ctx.getBlockIndex(f, b) >> 8) & 0xff;
  });
  
  _nextBlockIsSet = true;
  popPtr();
}


Slot Compiler::declareGlobal(std::string const &name, types::TypeHandle type) {
  error_if(not _state.allowGlobalDefinitions, "Global variable '", name, "' must be defined before the first function.");
  error_if(_program.isGlobal(name), "Multiple definitions of global variable '", name, "'.");

  int const offset = _program.globalVariableFrameSize();
  Slot slot {
    .name = name,
    .type = type,
    .kind = Slot::Global,
    .offset = offset,
    .scope = nullptr
  };

  _program.globals.emplace_back(slot);
  return slot;
}


Slot Compiler::declareLocal(std::string const& name, types::TypeHandle type) {
  assert(_currentFunction != nullptr);

  // Check if name is available in this scope
  auto &frame = _currentFunction->frame;
  bool available = true;
  for (auto const &slot: frame.locals) {
    if (slot.name == name && slot.scope == _currentScope) {
      available = false;
      break;
    }
  }
  assert(available && "variable with same name declared in this scope");
  return allocSlot(name, type, Slot::Local);
}


// TODO: does not belong in public interface
Slot Compiler::declareGlobalReference(Slot const &globalSlot) {
  assert(globalSlot.kind == Slot::Global);
  assert(_currentFunction != nullptr);
  assert(_currentBlock != nullptr && _currentBlock->name.starts_with("__prologue_"));
  
  FrameLayout &frame = _currentFunction->frame;
  int const offset = frame.localBase() + frame.localAreaSize();
  Slot slot {
    .name = std::string("__g_") + globalSlot.name,
    .type = globalSlot.type,
    .kind = Slot::GlobalReference,
    .offset = offset,
    .scope = _currentScope
  };
  frame.locals.emplace_back(std::move(slot));
  return frame.locals.back();
}


void Compiler::referGlobals(std::vector<std::string> const &names) {
  error_if(_currentFunction == nullptr, "called 'referGlobals' outside a function block");
  error_if(_currentBlock != nullptr, "called 'referGlobals' inside code-block");

  beginBlock(std::string("__prologue_") + _currentFunction->name); {

    std::unordered_set<std::string> declared;
    for (std::string const &name: names) {
      error_if(not _program.isGlobal(name), "in function '", _currentFunction->name, "': symbol '", name, "' was not declared as a global variable.");
      error_if(not declared.insert(name).second, "in function '", _currentFunction->name, "': symbol '", name, "' was referred to multiple times.");

      declareGlobalReference(_program.globalSlot(name));      
    }

    syncGlobalToLocal();
    setNextBlock(_program.nextGlobalBlockIndex());
  } endBlock();
}

// TODO: does not belong in public interface
Slot Compiler::local(std::string const& varName, bool globalReference) const {
  assert(_currentFunction != nullptr);

  Function::Scope *targetScope = _currentScope;
  while (true) {
    for (auto &slot: _currentFunction->frame.locals) {
      if (slot.name == varName && slot.scope == targetScope) {
	return slot;
      }
    }
    if (targetScope == nullptr) break;
    targetScope = targetScope->parent;
  }

  if (not globalReference) {
    std::string const globalReferenceName = std::string("__g_") + varName;
    return local(globalReferenceName, true);
  }

  error("variable '", varName, "' not declared in this scope (function '", _currentFunction, "').");
  std::unreachable();
}

Slot Compiler::getStructFieldImpl(values::LValue const &obj, int fieldIndex) {
  error_if(fieldIndex >= obj.type()->length(), "field index (", fieldIndex, ") out of bounds in call to getStructField.");
  return getStructFieldImpl(obj, obj.type()->fieldName(fieldIndex));
}

Slot Compiler::getStructFieldImpl(values::LValue const &obj, std::string const &fieldName) {
  Slot const slot = obj.slot();
  error_if(_currentFunction == nullptr, "called 'getStructField(", slot.name, ", ", fieldName, ")' outside function-block.");
  error_if(_currentBlock == nullptr, "called 'getStructField(", slot.name, ", ", fieldName, ")' outside code-block.");
  error_if(not types::isStruct(slot.type), "tried to call 'getStructField' on '", slot.name, "', which is not a struct.");

  auto s = static_cast<types::StructType const *>(slot.type);

  auto [fieldIndex, offset] = [&]() -> std::pair<size_t, size_t> {
    for (size_t idx = 0, offset = 0; idx != s->_fields.size(); ++idx) {
      types::StructType::Field const &f = s->_fields[idx];
      if (f.name == fieldName) return std::make_pair(idx, offset);
      offset += f.type->size();
    }
    return std::make_pair(-1, 0);
  }();
  
  error_if(fieldIndex == -1UL, "variable '", slot.name, "' of struct type '", slot.type->str(), "' does not contain field '", fieldName, "'.");

  return Slot {
    .name = std::string("__field_") + slot.name + "_" + fieldName,
    .type = s->_fields[fieldIndex].type,
    .kind = Slot::StructField,
    .offset = slot.offset + (int)offset
  };  
}

Slot Compiler::arrayElementConstImpl(values::LValue const &arr, int index) {
  Slot const slot = arr.slot();
  error_if(_currentFunction == nullptr, "called 'arrayElementConst(", slot.name, ", ", index, ")' outside function-block.");
  error_if(_currentBlock == nullptr, "called 'arrayElementConst(", slot.name, ", ", index, ")' outside code-block.");
  error_if(not types::isArray(slot.type) && not types::isString(slot.type),
	   "tried to call 'arrayElementConst' on '", slot.name, "', which is not an array.");
  error_if(index >= slot.type->length(), "index [", index, "] out of bounds for '", slot.name, "'.");

  // return a slot that represents the element 
  return Slot {
    .name = std::string("__elem_") + slot.name + "_" + std::to_string(index),
    .type = slot.type->elementType(),
    .kind = Slot::ArrayElement,
    .offset = slot.offset + (index * slot.type->elementType()->size())
  };
}


void Compiler::callFunctionImpl(std::string const& functionName, std::string const& nextBlockName,
				std::optional<values::LValue> const &returnSlot, std::vector<values::RValue> const &args) {

  error_if(_currentFunction == nullptr, "called 'callFunction(", functionName, ")' outside function-block.");
  error_if(_currentBlock == nullptr, "called 'callFunction(", functionName, ")' outside code-block.");
  assert(_currentSeq != nullptr);

  syncLocalToGlobal();

  std::string const metaBlockName = std::string("__ret_meta_") + _currentFunction->name + "_" + std::to_string(_metaBlocks.size());

  setNextBlock(_currentFunction->name, metaBlockName);
  _metaBlocks.push_back(MetaBlock{
      .name = metaBlockName,
      .caller = _currentFunction->name,
      .callee = functionName,
      .returnSlot = returnSlot ? std::optional<Slot>(returnSlot->slot()) : std::nullopt,
      .nextBlockName = nextBlockName,
    });

  deferFunctionCallTypeCheck(_currentFunction->name, functionName, args);
  copyArgsToNextFrame(functionName, args);
  pushFrame();
  setNextBlock(functionName, "");
}


void Compiler::abortProgram() {
  error_if(_currentFunction == nullptr, "called 'abortProgram' outside function-block.");
  error_if(_currentBlock == nullptr, "called 'abortProgram' outside code-block.");

  moveTo(FrameLayout::RunState, MacroCell::Value0);
  zeroCell();

  // Sync and pop
  popFrame();
  _nextBlockIsSet = true;
}


void Compiler::returnFromFunctionImpl(std::optional<values::RValue> const &ret) {
  if (ret) {
    error_if(_currentFunction == nullptr, "called 'returnFromFunction(", ret->str(),")' outside function-block.");
    error_if(_currentBlock == nullptr, "called 'returnFromFunction(", ret->str(),")' outside code-block.");
    error_if(ret->type() != _currentFunction->sig.returnType,
	     "type of return-value does not match function signature of '':"
	     "expected '", _currentFunction->sig.returnType->str(), "', got '", ret->type()->str(), "'.");

    // Copy the variable into the return-slot. TODO: non-globals can be moved rather than copied
    Slot returnSlot = {
      .name = "__return_slot",
      .type = ret->type(),
      .kind = Slot::Dummy,
      .offset = FrameLayout::ReturnValueStart,
      .scope = nullptr
    };
    
    assign(returnSlot, *ret);
  }
  else {
    error_if(_currentFunction == nullptr, "called 'returnFromFunction' outside function-block.");
    error_if(_currentBlock == nullptr, "called 'returnFromFunction' outside code-block.");
  }
  
  syncLocalToGlobal();
  popFrame();
  _nextBlockIsSet = true;  
}

void Compiler::assignImpl(values::LValue const &lhs, values::RValue const &rhs) {
  
  error_if(_currentFunction == nullptr, "called 'assign(", lhs.str(), ", ", rhs.str(), ")' outside function-block.");
  error_if(_currentBlock == nullptr, "called 'assign(",  lhs.str(), ", ", rhs.str(), ")' outside code-block.");
  error_if(not lhs.type()->isConstructibleFrom(rhs.type()),
	   "type mismatch in 'assign(", lhs.str(), ", ", rhs.str(), ")': '",
	   lhs.str(), "' is of type '", lhs.type()->str(), "' while '", rhs.str(), "' is of type '", rhs.type()->str(), "'.");

  pushPtr();
  
  // TODO: factor out each branch into a helper function
  if (rhs.hasSlot()) {
    Slot const dest = lhs.slot();
    Slot const src  = rhs.slot();
    types::TypeHandle const type = dest.type;
    
    // Copy src into dest
    for (int i = 0; i != type->size(); ++i) {
      moveTo(dest + i, MacroCell::Value0);
      zeroCell();
      moveTo(src + i, MacroCell::Value0);
      copyField(Cell{dest + i, MacroCell::Value0},
		Temps<1>::pack(dest + i, MacroCell::Scratch0));
      if (type->usesValue1()) {
	moveTo(dest + i, MacroCell::Value1);
	zeroCell();
	moveTo(src + i, MacroCell::Value1);
	copyField(Cell{dest + i, MacroCell::Value1},
		  Temps<1>::pack(dest + i, MacroCell::Scratch0));
      }
    }
  }
  else {
    // Constant -> construct in slot
    auto const constructInSlot = [&](auto&& self, Slot const &slot, values::Value const &val) -> void {
      
      if (types::isInteger(slot.type)) {
	int const x = val->value();
	moveTo(slot, MacroCell::Value0);
	setToValue(x & 0xff);
	if (slot.type->usesValue1()) {
	  moveTo(slot, MacroCell::Value1);
	  setToValue( (x >> 8) & 0xff);
	}
      }
      else if (types::isArray(slot.type) || types::isString(slot.type)) {
	for (int i = 0; i != val->type()->length(); ++i) {
	  self(self, arrayElementConst(slot, i), val->element(i));
	}
      }
      else if (types::isStruct(slot.type)) {
	// recursively call for each field
	for (int i = 0; i != val->type()->length(); ++i) {
	  self(self, getStructField(slot, i), val->field(i));
	}
      }
      else if (types::isPointer(slot.type)) {
	// first macrocell contains the frameDepth, second the offset
	// The framedepth is initialized to 0 for new pointers.

	int offset;
	if (types::isPointer(val->type())) {
	  // If the pointer is initialized with pointer-value,
	  // set its offset to the slot-offset of the variable pointed to.
	  Slot const pointeeSlot = local(val->pointee()->varName());
	  offset = pointeeSlot;
	}
	else {
	  // If the pointer is initialized with an integer,
	  // we can call value() on it to obtain the int value;
	  assert(types::isInteger(val->type()));
	  offset = val->value();
	}

	// Set frame-depth to 0 
	moveTo(slot + RuntimePointer::FrameDepth, MacroCell::Value0); zeroCell();
	// Construct offset in second cell
	moveTo(slot + RuntimePointer::Offset, MacroCell::Value0); setToValue(offset & 0xff);
	moveTo(slot + RuntimePointer::Offset, MacroCell::Value1); setToValue((offset >> 8) & 0xff);
      }
      else {
	assert(false && "not implemented");
      }
    };
    constructInSlot(constructInSlot, lhs.slot(), rhs.value());
  }

  popPtr();
}

void Compiler::writeOutImpl(values::RValue const &rhs) {
  error_if(_currentFunction == nullptr, "called 'writeOut(", rhs.str(), ")' outside function-block.");
  error_if(_currentBlock == nullptr, "called 'writeOut(",  rhs.str(), ")' outside code-block.");

  pushPtr();

  if (not rhs.hasSlot()) {
    Slot const tmp = getTemp(rhs.value());
    writeOut(rValue(tmp));
    popPtr();
    return;
  }

  Slot const slot = rhs.slot();
  if (types::isString(slot.type)){
    moveTo(slot, MacroCell::Value0);
    setSeekMarker();

    switchField(MacroCell::Value0);
    emit<primitive::CopyData>(MacroCell::Value0, MacroCell::Flag, MacroCell::Scratch0);
    switchField(MacroCell::Flag);
    loopOpen(); {
      zeroCell();
      switchField(MacroCell::Value0);
      emit<primitive::Out>();
      emit<primitive::MovePointerRelative>(MacroCell::FieldCount);

      // Check if end of string was reached by storing NOT(Value0) in Flag. If hit, flag0 becomes 0 and we exit the loop
      emit<primitive::CopyData>(MacroCell::Value0, MacroCell::Flag, MacroCell::Scratch0);
      switchField(MacroCell::Flag);
    } loopClose();

    // We hit the end of the string -> return to seek marker (no payload, don't skip current)
    seek(primitive::Left, 0, false);
    resetSeekMarker();
    switchField(MacroCell::Value0);
    
    // Back at slot -> DP is valid again
  }
  else {
    for (int i = 0; i != slot.type->size(); ++i) {
      moveTo(slot + i, MacroCell::Value0);
      emit<primitive::Out>();
      if (slot.type->usesValue1()) {
	moveTo(slot + i, MacroCell::Value1);
	emit<primitive::Out>();
      }
    }
  }

  popPtr();
}

Slot Compiler::deref(values::RValue const &ptr) {
  assert(false && "deref not implemented");
  
  // assert(ptr.type()->tag() == types::POINTER);
  
  // // Go to the frame/offset stored in the pointer. We need the data to
  // // be runtime, so if the RHS is a value, store it in a temp slot first.
  // Slot const ptrSlot = ptr.hasSlot() ? ptr.slot() : getTemp(ptr.value());

  // // Copy both values (frameDepth and offset) to the payload-cells of cell 0 and 1
  // moveTo(ptrSlot + RuntimePointer::FrameDepth, MacroCell::Value0);
  // copyField(RuntimePointer::FrameDepth, MacroCell::Payload0);

  // moveTo(ptrSlot + RuntimePointer::Offset, MacroCell::Value0);
  // copyField(0 + RuntimePointer::Offset, MacroCell::Payload0);
  // moveTo(ptrSlot + RuntimePointer::Offset, MacroCell::Value1);
  // copyField(0 + RuntimePointer::Offset, MacroCell::Payload1);

  // // Payload is now stored in cells 0 and 1 of the frame
  // // If the framedepth is nonzero, set the seek marker
  // moveTo(0 + RuntimePointer::FrameDepth, MacroCell::Payload0);
  // copyField(0 + RuntimePointer::FrameDepth, MacroCell::Flag);
  // moveTo(0 + RuntimePointer::FrameDepth, MacroCell::Flag);
  // loopOpen(); {
  //   zeroCell();
  //   moveToOrigin();
  //   setSeekMarker();
  //   moveTo(0 + RuntimePointer::FrameDepth, MacroCell::Flag);
  // } loopClose();

  // // If seekmarker is nonzero, go into a loop that jumps
  // // to the previous frame start until the depth-counter becomes 0.
  // moveTo(0 + RuntimePointer::FrameDepth, MacroCell::Payload0);
  // loopOpen(); {
  //   zeroCell();

  //   // TODO: this is moveToPreviousFrame with payload
  //   switchField(MacroCell::Flag);
  //   setToValue(1);
  //   loopOpen(); {
  //     zeroCell();

  //     // Copy depth and offset one cell to the left
  //     moveTo(0 + RuntimePointer::FrameDepth, MacroCell::Payload0);
  //     emit<primitive::MoveData>(-MacroCell::FieldCount);
  //     moveTo(0 + RuntimePointer::Offset, MacroCell::Payload0);
  //     emit<primitive::MoveData>(-MacroCell::FieldCount);
  //     moveTo(0 + RuntimePointer::Offset, MacroCell::Payload1);
  //     emit<primitive::MoveData>(-MacroCell::FieldCount);
  //     moveToOrigin();
      
  //     // Follow with the pointer and check if we landed on a frame-start
  //     emit<primitive::MovePointerRelative>(-MacroCell::FieldCount);
  //     switchField(MacroCell::FrameID);
  //     notValue(MacroCell::Flag);
  //     switchField(MacroCell::Flag);
  //   } loopClose();

  //   // We're now at the start of the previous frame -> exit if depth == 0 after subtracting 1
  //   moveTo(0 + RuntimePointer::FrameDepth, MacroCell::Payload0);
  //   subConst(1);    
  // } loopClose();

  // // We're at the target frame -> now move dynamically to the indicated offset

  // // Flag = payload0 || payload1
  // moveTo( RunTimePointer::Offset, MacroCell::Payload0);
  // orValues(RunTimePointer::Offset, MacroCell::Payload1,
  // 	   RuntimePointer::Offset, MacroCell::Flag);
  // switchField(MacroCell::Flag);
  // loopOpen(); {
  //   zeroCell();

  //   // Subtract 1 (in 16-bit mode) from the offset
  //   switchField(MacroCell::Payload0);
  //   subConst16(1, MacroCell::Payload1);

  //   // Copy new offset to next cell
  //   switchField(MacroCell::Payload0);
  //   emit<primitive::MoveData>(-MacroCell::FieldCount);
  //   switchField(MacroCell::Payload1);
  //   emit<primitive::MoveData>(-MacroCell::FieldCount);

  //   // Follow the payload and construct the flag
  //   emit<primitive::MovePointerRelative>(MacroCell::FieldCount);
  //   moveTo(RunTimePointer::Offset, MacroCell::Payload0);
  //   orValues(RunTimePointer::Offset, MacroCell::Payload1,
  // 	     RuntimePointer::Offset, MacroCell::Flag);
  //   switchField(MacroCell::Flag);
  // } loopClose();

  // // We're at the pointee (payload cells are both 0 now)
  // // Grab the value and move back to the seek marker
  // switchField(MacroCell::Value0);
  // copyField(0, MacroCell::Payload0);
  // if (ptr.type()->usesValue1()) {
  //   switchField(MacroCell::Value1);
  //   copyField(0, MacroCell::Payload1);
  // }

  // seek(primitives::Right, ptr.type()->usesValue1() ? 2 : 1);

  // // Back at origin of current frame -> move payload into slot
  // Slot const result = getTemp(ptr.type()->pointeeType());

  // switchField(MacroCell::Payload0);
  // moveField(result, MacroCell::Value0);
  // if (ptr.type()->usesValue1()) {
  //   switchField(MacroCell::Payload1);
  //   moveField(result, MacroCell::Value1);
  // }

  // return result;
}

