#include "compiler.ih"

void Compiler::setEntryPoint(std::string functionName) {
  _program.entryFunctionName = std::move(functionName);
}

void Compiler::begin() {
  error_if(_program.entryFunctionName.empty(),
	   "No entry point set before calling 'begin'; call 'setEntryPoint' first.");
  error_if(_state.begun, "called 'begin' before ending previous program.");

  _state.begun = true;

  // Globals should start at same frame offset as locals for consistency -> pad with raw
  declareGlobal("__pad__", TypeSystem::raw(FrameLayout::ReturnValueStart)); 
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
  // 1. Mark cell 0 using the SeekMarker field to indicate that this is where
  //    the global data frame starts, for easy navigation to this frame. 
  // 2. Move the pointer to the first stackframe and reset the origin.
  // 3. Initialize the first stack-frame, where we set the TargetBlock to the
  //    index of the entry-function and the run-state to 1. It is assigned
  //    FrameMarker 1, which from hereon will be copied and incremented for
  //    deeper frames.
  // 4. Open the main-loop and leave the pointer in cell 0 of the frame.

  setTargetSequence(&_program.bootstrap);

  resetOrigin();
  switchField(MacroCell::SeekMarker);
  setToValue(1);
  switchField(MacroCell::FrameMarker);
  setToValue(1);

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

SlotProxy Compiler::structFieldImpl(values::LValue const &obj, int fieldIndex) {
  auto structType = types::cast<types::StructType>(obj.type());
  error_if(fieldIndex >= structType->fieldCount(), "field index (", fieldIndex, ") out of bounds in call to structField.");
  return structFieldImpl(obj, structType->fieldName(fieldIndex));
}

SlotProxy Compiler::structFieldImpl(values::LValue const &obj, std::string const &fieldName) {
  error_if(_currentFunction == nullptr, "called 'structField(", obj.str(), ", ", fieldName, ")' outside function-block.");
  error_if(_currentBlock == nullptr, "called 'structField(", obj.str(), ", ", fieldName, ")' outside code-block.");
  error_if(not types::isStruct(obj.type()), "tried to call 'structField' on '", obj.str(), "', which is not a struct.");

  // TODO: make this check work (after fixing all the virtual functions of the types and adding a member for easy check)
  //  error_if(fieldIndex == -1UL, "variable '", obj.str(), "' of struct type '", obj.type()->str(), "' does not contain field '", fieldName, "'.");

  return proxy::structField(obj.slot(), fieldName);
}

SlotProxy Compiler::arrayElementImpl(values::LValue const &arr, int index) {
  error_if(_currentFunction == nullptr, "called 'arrayElementConst(", arr.str(), ", ", index, ")' outside function-block.");
  error_if(_currentBlock == nullptr, "called 'arrayElementConst(", arr.str(), ", ", index, ")' outside code-block.");
  error_if(not types::isArray(arr.type()) && not types::isString(arr.type()),
	   "tried to call 'arrayElementConst' on '", arr.str(), "', which is not an array.");
  error_if(index >= types::cast<types::ArrayLike>(arr.type())->length(), "index [", index, "] out of bounds for '", arr.str(), "'.");

  return proxy::arrayElement(arr.slot(), index);  
}  

SlotProxy Compiler::arrayElementImpl(values::LValue const &arr, values::RValue const &index) {

  error_if(_currentFunction == nullptr, "called 'arrayElement(", arr.str(), ", ", index.str(), ")' outside function-block.");
  error_if(_currentBlock == nullptr, "called 'arrayElement(", arr.str(), ", ", index.str(), ")' outside code-block.");
  error_if(not types::isArray(arr.type()) && not types::isString(arr.type()),
	   "tried to call 'arrayElement' on '", arr.str(), "', which is not an array.");
  error_if(not types::isInteger(index.type()), "tried to call 'arrayElement' with index '", index.str(), "' which is not an integer.");

  if (index.hasSlot()) return proxy::arrayElement(arr.slot(), index.slot());
  else return arrayElementImpl(arr, values::cast<types::IntegerType>(index.value())->value());  
}


SlotProxy Compiler::dereferencePointerImpl(values::RValue const &ptr) {
  error_if(_currentFunction == nullptr, "called 'dereferencePointer(", ptr.str(), ")' outside function-block.");
  error_if(_currentBlock == nullptr, "called 'dereferencePointer((",  ptr.str(), ")' outside code-block.");
  error_if(not types::isPointer(ptr.type()), "type mismatch in 'dereferencePointer(", ptr.str(), ")': '",
	   ptr.str(), "' is of type '", ptr.type()->str(), "', which is not a pointer.");

  return proxy::dereferencedPointer(ptr.hasSlot() ? ptr.slot() : getTemp(ptr.value()));
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
      .returnSlot = returnSlot ? std::optional<SlotProxy>(returnSlot->slot()) : std::nullopt,
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


void Compiler::copyElementIntoSlot(Slot const &elementSlot, Slot const &arrSlot, Slot const &indexSlot) {
  assert(types::isArray(arrSlot.type));
  assert(types::isInteger(indexSlot.type));
  assert(elementSlot.type == cast<types::ArrayLike>(arrSlot.type)->elementType());
  types::TypeHandle elementType = elementSlot.type;
  
  pushPtr();

  // TODO: use constructive version when I have it
  Slot const scaledIndexSlot = getTemp(TypeSystem::i8());
  assignSlot(scaledIndexSlot, indexSlot);
  moveTo(scaledIndexSlot, MacroCell::Value0);
  mulConst(elementType->size(),
	   Temps<3>::pack(scaledIndexSlot, MacroCell::Scratch0,
			  scaledIndexSlot, MacroCell::Scratch1,			  
			  scaledIndexSlot, MacroCell::Payload0
			  ));

  Payload payload(elementType->size(),
		  elementType->usesValue1() ? Payload::Width::Double : Payload::Width::Single);
  
  moveTo(arrSlot, MacroCell::Value0);
  setSeekMarker();
  fetchFromDynamicOffset(Cell{scaledIndexSlot, MacroCell::Value0},
			 Cell{scaledIndexSlot, MacroCell::Value1},
			 payload,
			 primitive::Left);

  for (int i = 0; i != elementType->size(); ++i) {
    moveTo(arrSlot + i, MacroCell::Payload0);
    moveField(Cell{elementSlot + i, MacroCell::Value0});
    if (elementType->usesValue1()) {
      moveTo(arrSlot + i, MacroCell::Payload1);
      moveField(Cell{elementSlot + i, MacroCell::Value1});
    }
  }
  
  moveTo(arrSlot);
  resetSeekMarker();
  popPtr();

}

void Compiler::copySlotIntoElement(Slot const &srcSlot, Slot const &arrSlot, Slot const &indexSlot) {
  assert(types::isArray(arrSlot.type));
  assert(types::isInteger(indexSlot.type));

  types::TypeHandle elementType = types::cast<types::ArrayLike>(arrSlot.type)->elementType();
  assert(srcSlot.type == elementType);

  pushPtr();

  Slot const scaledIndexSlot = getTemp(TypeSystem::i8());
  assignSlot(scaledIndexSlot, indexSlot);
  moveTo(scaledIndexSlot, MacroCell::Value0);
  mulConst(elementType->size(),
	   Temps<3>::pack(scaledIndexSlot, MacroCell::Scratch0,
			  scaledIndexSlot, MacroCell::Scratch1,			  
			  scaledIndexSlot, MacroCell::Payload0
			  ));
  
  // Plant a seek marker at the start of the array
  moveTo(arrSlot, MacroCell::Value0);
  setSeekMarker();

  // Plant another marker one (full element) beyond the start of the element we need
  goToDynamicOffset(Cell{scaledIndexSlot, MacroCell::Value0},
		    Cell{scaledIndexSlot, MacroCell::Value1});

  _dp.set(0);
  moveTo(elementType->size()); 
  setSeekMarker();
  moveTo(0);

  // Move back to the start of the array
  seek(MacroCell::SeekMarker, primitive::Left, {}, true);
  _dp.set(arrSlot);
  
  for (int i = 0; i != elementType->size(); ++i) {
    // Copy the contents into the payload cells
    moveTo(srcSlot + i, MacroCell::Value0);
    copyField(Cell{arrSlot + i, MacroCell::Payload0},
	      Temps<1>::pack(arrSlot + i, MacroCell::Scratch0));
    if (elementType->usesValue1()) {
      moveTo(srcSlot + i, MacroCell::Value1);
      copyField(Cell{arrSlot + i, MacroCell::Payload1},
		Temps<1>::pack(arrSlot + i, MacroCell::Scratch0));
    }
  }

  // Move the payload into the cell containing the marker (one beyond actual start of the element)
  moveTo(arrSlot);

  Payload payload(elementType->size(),
		  elementType->usesValue1() ? Payload::Width::Double : Payload::Width::Single);	  
  
  seek(MacroCell::SeekMarker, primitive::Right, payload, false);
  _dp.set(elementType->size());
    
  // Move the payload into the value-cells
  // Pointer value set to the start of the next element, so offset 0 represents the start of the target element
  for (int i = 0; i != elementType->size(); ++i) {
    moveTo(elementType->size() + i, MacroCell::Payload0);
    moveField(Cell{i, MacroCell::Value0});
    if (elementType->usesValue1()) {
      moveTo(elementType->size() + i, MacroCell::Payload1);
      moveField(Cell{i, MacroCell::Value1});
    }
  }

  moveTo(elementType->size());
  resetSeekMarker();

  // Go back to the start of the array
  seek(MacroCell::SeekMarker, primitive::Left, {}, false);
  _dp.set(arrSlot);
  resetSeekMarker();
  popPtr();
}

void Compiler::assignSlot(Slot const &dest, Slot const &src) {
  assert(dest.size() == src.size());

  // TODO: if src is a tmp we can move from it.
  
  pushPtr();
  // Copy src into slot
  for (int i = 0; i != dest.size(); ++i) {
    moveTo(src + i, MacroCell::Value0);
    copyField(Cell{dest + i, MacroCell::Value0},
	      Temps<1>::pack(dest + i, MacroCell::Scratch0));
    moveTo(src + i, MacroCell::Value1);
    if (dest.type->usesValue1()) {
      copyField(Cell{dest + i, MacroCell::Value1},
		Temps<1>::pack(dest + i, MacroCell::Scratch0));
    }
    else {
      setToValue(0);
    }
  }
  popPtr();
}

void Compiler::assignSlot(Slot const &slot, values::Anonymous const &val) {
  assert(slot.type->isConstructibleFrom(val->type()));

  pushPtr();
  if (types::isInteger(slot.type)) {
    int const x = values::cast<types::IntegerType>(val)->value();
    moveTo(slot, MacroCell::Value0);
    setToValue(x & 0xff);
    if (slot.type->usesValue1()) {
      moveTo(slot, MacroCell::Value1);
      setToValue((x >> 8) & 0xff);
    }
  }
  else if (types::isArray(slot.type) || types::isString(slot.type)) {
    // recursive call for each element
    for (int i = 0; i != types::cast<types::ArrayLike>(val->type())->length(); ++i) {
      arrayElement(slot, i)->write(*this, values::cast<types::ArrayLike>(val)->element(i));
    }
  }
  else if (types::isStruct(slot.type)) {
    // recursive call for each field	
    for (int i = 0; i != types::cast<types::StructType>(val->type())->fieldCount(); ++i) {
      structField(slot, i)->write(*this, values::cast<types::StructType>(val)->field(i));
    }
  }
  else if (types::isPointer(slot.type)) {
    // first macrocell contains the frameDepth, second the offset
    // The framedepth is initialized to 0 for new pointers.

    bool localPointer = true;
    int offset;	  
    if (types::isPointer(val->type())) {
      // If the pointer is initialized with pointer-value,
      // set its offset to the slot-offset of the variable pointed to.
      auto pointerType = types::cast<types::PointerType>(val->type());
      auto pointeeType = pointerType->pointeeType();
      auto pointerVal = values::cast<types::PointerType>(val);
      
      Slot const localSlot = local(pointerVal->pointee()->varName());
      if (localSlot.name.starts_with("__g_")) {
	std::string const globalName = localSlot.name.substr(std::string("__g_").size());
	assert(_program.isGlobal(globalName));
	Slot const globalSlot = _program.globalSlot(globalName);
	assert(globalSlot.type == pointeeType);
	
	offset = globalSlot.offset + pointeeType->size() * pointerVal->offset();
	localPointer = false;
	_addressTakenGlobals.insert(globalName);
      }
      else {
	offset = localSlot.offset + pointeeType->size() * pointerVal->offset();
      }
    }
    else {
      // If the pointer is initialized with an integer,
      // we can call value() on it to obtain the int value. This will be
      // interpreted as an address.
      assert(types::isInteger(val->type()));
      offset = values::cast<types::IntegerType>(val)->value();
    }

    // Set frame-depth to 0 for a local pointer, FrameID for a global pointer
    if (localPointer) {
      moveTo(slot + RuntimePointer::FrameDepth, MacroCell::Value0);
      zeroCell();
    } else {
      moveTo(0, MacroCell::FrameMarker);
      copyField(Cell{slot + RuntimePointer::FrameDepth, MacroCell::Value0},
		Temps<1>::pack(slot + RuntimePointer::FrameDepth, MacroCell::Scratch0));

    }

    // Construct offset in second cell
    moveTo(slot + RuntimePointer::Offset, MacroCell::Value0);
    setToValue(offset & 0xff);
    moveTo(slot + RuntimePointer::Offset, MacroCell::Value1);
    setToValue((offset >> 8) & 0xff);
  }
  else {
    assert(false && "not implemented");
  }
  popPtr();
}


void Compiler::assignImpl(values::LValue const &lhs, values::RValue const &rhs) {
  error_if(_currentFunction == nullptr, "called 'assign(", lhs.str(), ", ", rhs.str(), ")' outside function-block.");
  error_if(_currentBlock == nullptr, "called 'assign(",  lhs.str(), ", ", rhs.str(), ")' outside code-block.");
  error_if(not lhs.type()->isConstructibleFrom(rhs.type()),
	   "type mismatch in 'assign(", lhs.str(), ", ", rhs.str(), ")': '",
	   lhs.str(), "' is of type '", lhs.type()->str(), "' while '", rhs.str(), "' is of type '", rhs.type()->str(), "'.");

  rhs.hasSlot()
    ? lhs.slot()->write(*this, rhs.slot())
    : lhs.slot()->write(*this, rhs.value());
}



void Compiler::writeOutImpl(values::RValue const &rhs) {
  error_if(_currentFunction == nullptr, "called 'writeOut(", rhs.str(), ")' outside function-block.");
  error_if(_currentBlock == nullptr, "called 'writeOut(",  rhs.str(), ")' outside code-block.");

  pushPtr();

  Slot const slot = rhs.hasSlot()
    ? rhs.slot()->materialize(*this)
    : getTemp(rhs.value());

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
    seek(MacroCell::SeekMarker, primitive::Left, {}, true);
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


// TODO: factor common code out from these deref functions

void Compiler::writeSlotThroughDereferencedPointer(Slot const &ptrSlot, Slot const &srcSlot) {
  assert(types::isPointer(ptrSlot.type));
  assert(srcSlot.type == types::cast<types::PointerType>(ptrSlot.type)->pointeeType());

  pushPtr();

  // Decompose the pointer into its frameDepth and offset
  Cell const frameDepth { ptrSlot + RuntimePointer::FrameDepth, MacroCell::Value0 };
  Cell const offsetLow  { ptrSlot + RuntimePointer::Offset, MacroCell::Value0 };
  Cell const offsetHigh { ptrSlot + RuntimePointer::Offset, MacroCell::Value1 };

  // Payload cells start at the origin. First, the pointer-fields
  // TODO: just load payload into ptrSlot payload and move dynamically to FrameMarker?
  Cell const frameDepthPayload { 0 + RuntimePointer::FrameDepth, MacroCell::Payload0 };
  Cell const offsetLowPayload  { 0 + RuntimePointer::Offset, MacroCell::Payload0 };
  Cell const offsetHighPayload { 0 + RuntimePointer::Offset, MacroCell::Payload1 };
  
  // Copy pointer (frameDepth and offset) to the payload-cells of cell 0 and 1
  moveTo(frameDepth);
  copyField(frameDepthPayload, Temps<1>::pack(frameDepthPayload, MacroCell::Scratch0));

  moveTo(offsetLow);
  copyField(offsetLowPayload,  Temps<1>::pack(offsetLowPayload, MacroCell::Scratch0));

  moveTo(offsetHigh);
  copyField(offsetHighPayload, Temps<1>::pack(offsetHighPayload, MacroCell::Scratch0));

  // Leave a marker at the sourceSlot
  moveTo(srcSlot);
  setSeekMarker();

  // If frameDepth is nonzero, we need to keep moving to the 
  // previous frame start until the depth-counter becomes 0.
  moveTo(frameDepthPayload);
  loopOpen(); {
    Payload payload{
      1, Payload::Width::Single, // depth
      1, Payload::Width::Double, // offset
    };
    moveToPreviousFrame(payload);
    
    // We're now at the start of the previous frame -> exit if depth == 0 after subtracting 1
    moveTo(frameDepthPayload);
    subConst(1); // TODO: dec()
  } loopClose();

  // At the target frame -> move to offset indicated by pointer value in payload
  goToDynamicOffset(offsetLowPayload, offsetHighPayload);

  // Set the marker and move back to the source
  setSeekMarker();
  seek(MacroCell::SeekMarker, primitive::Right, {}, false);
  _dp.set(srcSlot);
  
  // Copy contents of the source-slot into the payload
  for (int i = 0; i != srcSlot.size(); ++i) {
    moveTo(srcSlot + i, MacroCell::Value0);
    copyField(Cell{srcSlot + i, MacroCell::Payload0},
	      Temps<1>::pack(srcSlot + i, MacroCell::Scratch0));
    if (srcSlot.type->usesValue1()) {
    moveTo(srcSlot + i, MacroCell::Value1);
    copyField(Cell{srcSlot + i, MacroCell::Payload1},
	      Temps<1>::pack(srcSlot + i, MacroCell::Scratch0));
    }
  }

  // Seek back to the pointee's slot
  moveTo(srcSlot);
  Payload payload{
    srcSlot.size(),
    srcSlot.type->usesValue1() ? Payload::Width::Double : Payload::Width::Single
  };
  
  seek(MacroCell::SeekMarker, primitive::Left, payload, false);
  resetSeekMarker();
  _dp.set(0);
  
  // Move contents of the payload in the slot
  for (int i = 0; i != srcSlot.size(); ++i) {
    moveTo(i, MacroCell::Payload0);
    moveField(Cell{i, MacroCell::Value0});
    if (srcSlot.type->usesValue1()) {
      moveTo(i, MacroCell::Payload1);
      moveField(Cell{i, MacroCell::Value1});
    }
  }

  // Seek back to the source
  seek(MacroCell::SeekMarker, primitive::Right, {}, false);
  resetSeekMarker();
  _dp.set(srcSlot);
  
  popPtr();

  syncGlobalToLocal(true);
}

void Compiler::dereferencePointerIntoSlot(Slot const &ptrSlot, Slot const &derefSlot) {
  assert(types::isPointer(ptrSlot.type));
  assert(derefSlot.type == types::cast<types::PointerType>(ptrSlot.type)->pointeeType());

  syncLocalToGlobal(true);

  
  pushPtr();

  // Decompose the pointer into its frameDepth and offset
  Cell const frameDepth { ptrSlot + RuntimePointer::FrameDepth, MacroCell::Value0 };
  Cell const offsetLow  { ptrSlot + RuntimePointer::Offset, MacroCell::Value0 };
  Cell const offsetHigh { ptrSlot + RuntimePointer::Offset, MacroCell::Value1 };

  // Payload cells will be at the origin
  // TODO: just load payload into ptrSlot payload and move dynamically to FrameMarker?
  Cell const frameDepthPayload { 0 + RuntimePointer::FrameDepth, MacroCell::Payload0 };
  Cell const offsetLowPayload  { 0 + RuntimePointer::Offset, MacroCell::Payload0 };
  Cell const offsetHighPayload { 0 + RuntimePointer::Offset, MacroCell::Payload1 };

  // Copy both values (frameDepth and offset) to the payload-cells of cell 0 and 1
  moveTo(frameDepth);
  copyField(frameDepthPayload, Temps<1>::pack(frameDepthPayload, MacroCell::Scratch0));

  moveTo(offsetLow);
  copyField(offsetLowPayload,  Temps<1>::pack(offsetLowPayload, MacroCell::Scratch0));

  moveTo(offsetHigh);
  copyField(offsetHighPayload, Temps<1>::pack(offsetHighPayload, MacroCell::Scratch0));

  // Leave a marker at the destination
  moveTo(derefSlot);
  setSeekMarker();

  // If frameDepth is nonzero, we need to keep moving to the 
  // previous frame start until the depth-counter becomes 0.
  moveTo(frameDepthPayload);
  loopOpen(); {
    Payload payload{
      1, Payload::Width::Single, // depth
      1, Payload::Width::Double, // offset
    };
    moveToPreviousFrame(payload);
    
    // We're now at the start of the previous frame -> exit if depth == 0 after subtracting 1
    moveTo(frameDepthPayload);
    subConst(1);
  } loopClose();

  // At the target frame -> move to offset indicated by pointer value in payload
  goToDynamicOffset(offsetLowPayload, offsetHighPayload);
  _dp.set(0);

  // Copy the value into the payload
  pushPtr();
  for (int i = 0; i != derefSlot.size(); ++i) {
    moveTo(i, MacroCell::Value0);
    copyField(Cell{i, MacroCell::Payload0}, Temps<1>::pack(i, MacroCell::Scratch0));
    if (derefSlot.type->usesValue1()) {
      moveTo(i, MacroCell::Value1);    
      copyField(Cell{i, MacroCell::Payload1}, Temps<1>::pack(i, MacroCell::Scratch0));
    }
  }
  popPtr();
  
  // Seek back to the start of the frame, then to the seekmarker left behind
  // at the deref-slot. We can't seek to the seekmarker directly because
  // we might have ended up to the right of it, when we're still in the target frame.

  Payload payload {
    derefSlot.size(),
    derefSlot.type->usesValue1() ? Payload::Width::Double : Payload::Width::Single
  };

  seek(MacroCell::FrameMarker, primitive::Left, payload, true); // TODO: this can become false again now that the padding was added to the global frame
  seek(MacroCell::SeekMarker, primitive::Right, payload, false);
  resetSeekMarker();
  
  // We're now at the marker that marks the deref-slot -> need to rebase.
  _dp.set(derefSlot);

  // Move payload into value-cells
  for (int i = 0; i != derefSlot.size(); ++i) {
    moveTo(derefSlot + i, MacroCell::Payload0);
    moveField(Cell{derefSlot + i, MacroCell::Value0});
    if (derefSlot.type->usesValue1()) {
      moveTo(derefSlot + i, MacroCell::Payload1);    
      moveField(Cell{derefSlot + i, MacroCell::Value1});
    } else {
      moveTo(derefSlot + i, MacroCell::Value1);
      zeroCell();
    }
  }
  
  popPtr();
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

