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
  switchField(MacroCell::FrameMarker);
  setToValue(1);

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
  error_if(_currentFunction == nullptr, "called 'arrayElementConst(", arr.str(), ", ", index, ")' outside function-block.");
  error_if(_currentBlock == nullptr, "called 'arrayElementConst(", arr.str(), ", ", index, ")' outside code-block.");
  error_if(not types::isArray(slot.type) && not types::isString(slot.type),
	   "tried to call 'arrayElementConst' on '", arr.str(), "', which is not an array.");
  error_if(index >= slot.type->length(), "index [", index, "] out of bounds for '", arr.str(), "'.");

  // return a slot that represents the element 
  return Slot {
    .name = std::string("__elem_") + slot.name + "_" + std::to_string(index),
    .type = slot.type->elementType(),
    .kind = Slot::ArrayElement,
    .offset = slot.offset + (index * slot.type->elementType()->size())
  };
}

Slot Compiler::arrayElementImpl(values::LValue const &arr, values::RValue const &index, std::optional<values::LValue> const &dest) {
  Slot const slot = arr.slot();
  error_if(_currentFunction == nullptr, "called 'arrayElement(", arr.str(), ", ", index.str(), ")' outside function-block.");
  error_if(_currentBlock == nullptr, "called 'arrayElement(", arr.str(), ", ", index.str(), ")' outside code-block.");
  error_if(not types::isArray(slot.type) && not types::isString(slot.type),
	   "tried to call 'arrayElement' on '", arr.str(), "', which is not an array.");
  error_if(not types::isInteger(index.type()), "tried to call 'arrayElement' with index '", index.str(), "' which is not an integer.");
  
  if (not index.hasSlot()) {
    return arrayElementConstImpl(arr, index.value()->value());
  }


  types::TypeHandle elementType = arr.type()->elementType();
  Slot const indexSlot = index.slot();
  Slot const destSlot = dest.has_value() ? dest->slot() : getTemp(elementType);

  // TODO: multiply index by size of element -> implement mulConst and mul16Const
  
  // We need to do a dynamic fetch:
  // Set a marker at the start of the array and fetch the value into the payload-fields.
  // Then move the value into the destination slot.
  moveTo(slot, MacroCell::Value0);
  setSeekMarker();  
  for (int i = 0; i != elementType->size(); ++i) {
    fetchFromDynamicOffset(Cell{indexSlot, MacroCell::Value0},
			   Cell{indexSlot, MacroCell::Value1},
			   i,
			   elementType->usesValue1() ? Payload::Double : Payload::Single,
			   primitive::Left);

    switchField(MacroCell::Payload0);
    moveField(Cell{destSlot + i, MacroCell::Value0});
    if (elementType->usesValue1()) {
      switchField(MacroCell::Payload1);
      moveField(Cell{destSlot + i, MacroCell::Value1});
    }
  }
  
  resetSeekMarker();
  return destSlot;
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
  initializeArguments(functionName, args);
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
      moveTo(src + i, MacroCell::Value0);
      copyField(Cell{dest + i, MacroCell::Value0},
		Temps<1>::pack(dest + i, MacroCell::Scratch0));
      if (type->usesValue1()) {
	moveTo(src + i, MacroCell::Value1);
	copyField(Cell{dest + i, MacroCell::Value1},
		  Temps<1>::pack(dest + i, MacroCell::Scratch0));
      }
    }
  }
  else {
    
    // RHS is a value -> construct in slot
    auto const constructInSlot = [&](auto&& self, Slot const &slot, values::Value const &val) -> void {
      assert(slot.type->isConstructibleFrom(val->type()));
      
      if (types::isInteger(slot.type)) {
	int const x = val->value();
	moveTo(slot, MacroCell::Value0);
	setToValue(x & 0xff);
	if (slot.type->usesValue1()) {
	  moveTo(slot, MacroCell::Value1);
	  setToValue((x >> 8) & 0xff);
	}
      }
      else if (types::isArray(slot.type) || types::isString(slot.type)) {
	// recursive call for each element
	for (int i = 0; i != val->type()->length(); ++i) {
	  self(self, arrayElementConst(slot, i), val->element(i));
	}
      }
      else if (types::isStruct(slot.type)) {
	// recursive call for each field	
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
	  offset = local(val->pointee()->varName()).offset;
	}
	else {
	  // If the pointer is initialized with an integer,
	  // we can call value() on it to obtain the int value. This will be
	  // interpreted as an address.
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
    writeOut(rValue(getTemp(rhs.value())));
    popPtr();
    return;
  }

  Slot const slot = rhs.slot();

  // Special case for Strings: look for NULL terminator
  if (types::isString(slot.type)) {
    moveTo(slot, MacroCell::Value0);
    setSeekMarker();

    emit<primitive::CopyData>(MacroCell::Value0, MacroCell::Flag, MacroCell::Scratch0);
    switchField(MacroCell::Flag);
    loopOpen(); {
      zeroCell();
      switchField(MacroCell::Value0);
      emit<primitive::Out>();
      emit<primitive::MovePointerRelative>(MacroCell::FieldCount);

      // Check if end of string was reached by using the current value as a flag.
      // If NULL terminator hit, we exit the loop and go back to start.
      emit<primitive::CopyData>(MacroCell::Value0, MacroCell::Flag, MacroCell::Scratch0);
      switchField(MacroCell::Flag);
    } loopClose();

    // We hit the end of the string -> return to seek marker (no payload, check current as well)
    seek(MacroCell::SeekMarker, primitive::Left, Payload::None, true);
    resetSeekMarker();
    popPtr();
    return;
  }

  // All other types: just output all Values sequentially
  for (int i = 0; i != slot.type->size(); ++i) {
    moveTo(slot + i, MacroCell::Value0);
    emit<primitive::Out>();
    if (slot.type->usesValue1()) {
      moveTo(slot + i, MacroCell::Value1);
      emit<primitive::Out>();
    }
  }
  popPtr();
}

Slot Compiler::deref(values::RValue const &ptr) {
  assert(false && "deref not implemented");
}  
// assert(ptr.type()->tag() == types::POINTER);
  
// // Go to the frame/offset stored in the pointer. We need the data to
// // be runtime, so if the RHS is a value, store it in a temp slot first.
// Slot const ptrSlot = ptr.hasSlot() ? ptr.slot() : getTemp(ptr.value());

// Cell const frameDepth { ptrSlot + RuntimePointer::FrameDepth, MacroCell::Value0 };
// Cell const offsetLow  { ptrSlot + RuntimePointer::Offset, MacroCell::Value0 };
// Cell const offsetHigh { ptrSlot + RuntimePointer::Offset, MacroCell::Value1 };

// Cell const frameDepthPayload { 0 + RuntimePointer::FrameDepth, MacroCell::Payload0 };
// Cell const offsetLowPayload  { 0 + RuntimePointer::Offset, MacroCell::Payload0 };
// Cell const offsetHighPayload { 0 + RuntimePointer::Offset, MacroCell::Payload1 };
    
// // Copy both values (frameDepth and offset) to the payload-cells of cell 0 and 1
// moveTo(frameDepth);
// copyField(frameDepthPayload, Temps<1>::pack(0 + RuntimePointer::FrameDepth, MacroCell::Scratch0));

// moveTo(offsetLow);
// copyField(offsetLowPayload,  Temps<1>::pack(0 + RuntimePointer::Offset, MacroCell::Scratch0));

// moveTo(offsetHigh);
// copyField(offsetHighPayload, Temps<1>::pack(0 + RuntimePointer::Offset, MacroCell::Scratch0));

// // Payload is now stored in cells 0 and 1 of the frame
// // If the framedepth is nonzero, set the seek marker
// Cell const frameDepthFlag { 0 + RuntimePointer::FrameDepth, MacroCell::Flag };
// moveTo(frameDepthPayload);
// copyField(frameDepthFlag);
// moveTo(frameDepthFlag);
// loopOpen(); {
//   zeroCell();
//   moveToOrigin();
//   setSeekMarker();
//   moveTo(0 + RuntimePointer::FrameDepth, MacroCell::Flag);
// } loopClose();

// // If frameDepth is nonzero, we need to keep moving to the 
// // previous frame start until the depth-counter becomes 0.
// moveTo(frameDepthPayload);
// loopOpen(); {

//   // TODO: need to drag multiple payload cells. However we're not even sure
//   // that in the other frame, there are two payload cells available. Is this ok?
//   // Frames might contain only one logical cell. Is it ok when the payload overlaps?
//   // Probably...
//   moveToPreviousFrame(/* ????? */);
    
//   // We're now at the start of the previous frame -> exit if depth == 0 after subtracting 1
//   moveTo(frameDepthPayload);
//   subConst(1);
// } loopClose();

// // We're at the target frame -> now move dynamically to the indicated offset

