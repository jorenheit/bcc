#include "compiler.ih"

void Compiler::setEntryPoint(std::string functionName) {
  _program.entryFunctionName = std::move(functionName);
}

void Compiler::begin() {
  assert(not _program.entryFunctionName.empty()); 
}

void Compiler::end() {
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
  _dp.setField(static_cast<MacroCell::Field>(0));
  switchField(MacroCell::SeekMarker);
  addConst(1);
  switchField(MacroCell::Value0);
  moveTo(1 + _program.globalVariableFrameSize()); 

  resetOrigin();
  switchField(MacroCell::FrameID);
  addConst(FrameLayout::FirstStackFrameID);

  setNextBlock(_program.entryFunctionName);
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

void Compiler::beginFunction(std::string const &name, types::TypePtr returnType) {
  assert(_currentFunction == nullptr);
  _currentFunction = &_program.createFunction(name, returnType);
}

void Compiler::beginFunction(std::string const &name) {
  _currentFunction = &_program.createFunction(name, _ts.voidT());
}

void Compiler::endFunction() {
  assert(_currentFunction != nullptr);
  assert(_currentBlock == nullptr);
  _currentFunction = nullptr;
}

void Compiler::beginBlock(std::string name) {
  assert(_currentFunction != nullptr);
  assert(_currentBlock == nullptr);

  auto globalIdx = _program.nextGlobalBlockIndex();
  Function::Block &block = _currentFunction->createBlock(std::move(name), globalIdx);
  _program.registerBlock(block);

  if (_currentFunction->blocks.size() == 1) {
    _currentFunction->entryBlockIndex = globalIdx;
  }

  _currentBlock = &block;
  _dp.resetTo(0);
  _nextBlockIsSet = false;

  setTargetSequence(&block.code);
  blockOpen();
}

void Compiler::endBlock() {
  assert(_currentBlock != nullptr);
  assert(_dp.isStatic());
  assert(_nextBlockIsSet);
      
  blockClose();
  _currentBlock = nullptr;
}

void Compiler::setNextBlock(int index) {
  assert(_currentSeq != nullptr);

  pushPtr();

  int const low = index & 0xff;
  int const high = (index >> 8) & 0xff;
  
  moveTo(FrameLayout::TargetBlock, MacroCell::Value0);
  setToValue(low);
  moveTo(FrameLayout::TargetBlock, MacroCell::Value1);
  setToValue(high);
  
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


Slot &Compiler::declareGlobal(std::string const &name, types::TypePtr type) {
  // TODO: very similar to declareLocal, could probably be refactored nicely
  assert(_currentFunction == nullptr);
  assert(!_program.globals.contains(name));

  int const offset = _program.globalVariableFrameSize();
  Slot slot {
    .name = name,
    .type = type,
    .storageType = Slot::Global,
    .offset = offset
  };

  auto [it, success] = _program.globals.emplace(name, std::move(slot));
  assert(success);
  return it->second;
}


Slot &Compiler::declareLocal(std::string const& name, types::TypePtr type) {
  assert(_currentFunction != nullptr);
  assert(!_currentFunction->frame.locals.contains(name));

  FrameLayout &frame = _currentFunction->frame;
  int const offset = frame.localBase() + frame.localAreaSize();
  Slot slot {
    .name = name,
    .type = type,
    .storageType = Slot::Local,
    .offset = offset
  };

  auto [it, success] = frame.locals.emplace(name, std::move(slot));
  assert(success);
  return it->second;
}


Slot &Compiler::declareGlobalReference(Slot const &globalSlot) {
  assert(globalSlot.storageType == Slot::Global);
  assert(_currentFunction != nullptr);
  assert(_currentBlock != nullptr && _currentBlock->name.starts_with("__prologue_"));
  
  FrameLayout &frame = _currentFunction->frame;
  int const offset = frame.localBase() + frame.localAreaSize();
  Slot slot {
    .name = std::string("__g_") + globalSlot.name,
    .type = globalSlot.type,
    .storageType = Slot::GlobalReference,
    .offset = offset
  };

  auto [it, success] = frame.locals.emplace(slot.name, std::move(slot));
  assert(success);
  return it->second;
}


void Compiler::referGlobals(std::vector<std::string> const &names) {
  assert(_currentFunction != nullptr);
  assert(_currentBlock == nullptr);

  beginBlock(std::string("__prologue_") + _currentFunction->name); {
    for (std::string const &name: names) {
      assert(_program.globals.contains(name));

      Slot const &globalSlot = _program.globals.at(name);
      declareGlobalReference(globalSlot);
    }

    syncGlobalToLocal();
    setNextBlock(_program.nextGlobalBlockIndex());
  } endBlock();
}
    
Slot &Compiler::local(std::string const& name) {
  assert(_currentFunction != nullptr);
  auto &locals = _currentFunction->frame.locals;
  auto it = locals.find(name);
  if (it == locals.end()) {
    it = locals.find(std::string("__g_") + name);
  }
  assert(it != locals.end());
  return it->second;
}

Slot &Compiler::global(std::string const& name) {
  assert(false && "I don't think this should be used.");
  assert(_currentFunction != nullptr);
  auto &globals = _program.globals;
  auto it = globals.find(name);
  assert(it != globals.end());
  return it->second;
}

Slot Compiler::arrayElementConst(std::string const &name, int index) {
  assert(_currentFunction != nullptr);

  // Check to see if this is an actual array and if the index is within bounds
  Slot const &arraySlot = local(name);
  assert(arraySlot.type->isArray());
  auto const *array = types::cast<types::ArrayType>(arraySlot.type);
  assert(index < array->length);

  // Create a new slot that represents the element
  return Slot {
    .name = std::string("__elem_") + name + "_" + std::to_string(index),
    .type = array->elementType,
    .storageType = Slot::ArrayElement,
    .offset = arraySlot.offset + (index * array->elementType->size())
  };
}


void Compiler::callFunction(std::string const &functionName,
			    std::string const &nextBlockName,
			    std::string const &returnVar) {
  assert(_currentFunction != nullptr);
  assert(_currentBlock != nullptr);
  assert(_currentSeq != nullptr);

  syncLocalToGlobal();

  auto const metaBlockName = std::string("__ret_meta_") 
    + _currentFunction->name + "_"
    + std::to_string(_metaBlocks.size());


  setNextBlock(_currentFunction->name, metaBlockName);
  _metaBlocks.push_back({
      .name = metaBlockName,
      .caller = _currentFunction->name,
      .callee = functionName,
      .returnVar = returnVar,
      .nextBlockName = nextBlockName,
    });

  pushFrame();
  setNextBlock(functionName);
}
  

void Compiler::abortProgram() {
  assert(_currentBlock != nullptr);

  moveTo(FrameLayout::RunState, MacroCell::Value0);
  zeroCell();

  // Sync and pop
  popFrame();
  _nextBlockIsSet = true;
}

void Compiler::returnConstFromFunction(int value) {
  assert(_currentBlock != nullptr);

  // On return:
  // 1. Populate return-value slot
  // 2. Sync globals. TODO: skip read-only globals
  // 3. Restore caller's frame (move pointer back until it hits the frame-marker)

  // Check if return-type is single integer
  assert(_currentFunction->returnType->isInteger());

  // Populate the return-value slot
  moveTo(FrameLayout::ReturnValueStart, MacroCell::Value0);
  setToValue(value & 0xff);
  if (_currentFunction->returnType->usesValue1()) {
    moveTo(FrameLayout::ReturnValueStart, MacroCell::Value1);
    setToValue((value >> 8) & 0xff);
  }  
  
  // Sync and pop
  syncLocalToGlobal();
  popFrame();
  _nextBlockIsSet = true;
}


void Compiler::returnFromFunction(std::string const &varName) {
  assert(_currentBlock != nullptr);

  // On return:
  // 1. Populate return-value slot
  // 2. Sync globals. TODO: skip read-only globals
  // 3. Restore caller's frame (move pointer back until it hits the frame-marker)

  if (not varName.empty()) {
    // Check if return variable matches the function's type
    Slot const &slot = local(varName);
    assert(slot.type == _currentFunction->returnType); // TODO: convert API-level asserts exceptions/errors

    // Copy the variable into the return-slot. TODO: non-globals can be moved rather than copied
    for (int i = 0; i != slot.type->size(); ++i) {
      moveTo(FrameLayout::ReturnValueStart + i, MacroCell::Value0);
      zeroCell();
      moveTo(slot + i, MacroCell::Value0);
      copyField(FrameLayout::ReturnValueStart + i, MacroCell::Value0);
      if (slot.type->usesValue1()) {
	moveTo(FrameLayout::ReturnValueStart + i, MacroCell::Value1);
	zeroCell();
	moveTo(slot + i, MacroCell::Value1);
	copyField(FrameLayout::ReturnValueStart + i, MacroCell::Value1);
      }
    }
  }
  else {
    assert(_currentFunction->returnType == _ts.voidT());
  }
  
  // Sync and pop
  syncLocalToGlobal();
  popFrame();
  _nextBlockIsSet = true;
}

void Compiler::assignConst(std::string const &var, int value) {
  Slot const &slot = local(var);
  assert(slot.type->isInteger());

  moveTo(slot, MacroCell::Value0);
  setToValue(value & 0xff);
  if (slot.type->usesValue1()) {
    moveTo(slot, MacroCell::Value1);
    setToValue( (value >> 8) & 0xff);
  }
}

void Compiler::assignConst(int offset, int value) {
  int const low = value & 0xff;
  int const high = (value >> 8) & 0xff;
  moveTo(offset, MacroCell::Value0);
  setToValue(low);
  moveTo(offset, MacroCell::Value1);
  setToValue(high);
}

void Compiler::writeOut(std::string const &var) {
  writeOut(local(var));
}

void Compiler::writeOut(Slot const &slot) {
  for (int i = 0; i != slot.type->size(); ++i) {
    moveTo(slot + i, MacroCell::Value0);
    emit<primitive::Out>();
    if (slot.type->usesValue1()) {
      moveTo(slot + i, MacroCell::Value1);
      emit<primitive::Out>();
    }
  }
}

