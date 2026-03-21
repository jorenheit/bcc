#include "compiler.ih"

void Compiler::setEntryPoint(std::string functionName) {
  _program.entryFunctionName = std::move(functionName);
}

void Compiler::begin() {
  assert(not _program.entryFunctionName.empty()); 
}

void Compiler::end() {
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


void Compiler::beginFunction(std::string const &name, FunctionSignature const &sig) {
  assert(_currentFunction == nullptr);
  assert(_currentScope == nullptr);
  assert(_currentBlock == nullptr);
  
  _currentFunction = &_program.createFunction(name, sig);  
  for (FunctionParam const &p: sig.params) {
    declareLocal(p.name, p.type);
  }

}

void Compiler::endFunction() {
  assert(_currentFunction != nullptr);
  assert(_currentBlock == nullptr);
  assert(_currentScope == nullptr);
  
  _currentFunction = nullptr;
}

void Compiler::beginScope() {
  assert(_currentFunction != nullptr);
  assert(_currentBlock == nullptr);
  
  _currentScope = &_currentFunction->createScope(_currentScope);
}

void Compiler::endScope() {
  assert(_currentFunction != nullptr);
  assert(_currentScope != nullptr);
  
  freeScope(_currentScope);
  _currentScope = _currentScope->parent;
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


Slot Compiler::declareGlobal(std::string const &name, types::TypeHandle type) {
  assert(_currentFunction == nullptr);
  assert(!_program.globals.contains(name));

  int const offset = _program.globalVariableFrameSize();
  Slot slot {
    .name = name,
    .type = type,
    .kind = Slot::Global,
    .offset = offset,
    .scope = nullptr
  };

  auto [it, success] = _program.globals.emplace(name, std::move(slot));
  assert(success);
  return it->second;
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
    
Slot Compiler::local(std::string const& varName, bool globalReference) {
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

  assert(false && "variable not in scope");
  std::unreachable();
}

Slot Compiler::global(std::string const& name) {
  assert(false && "I don't think this should be used.");
  assert(_currentFunction != nullptr);
  auto &globals = _program.globals;
  auto it = globals.find(name);
  assert(it != globals.end());
  return it->second;
}

Slot Compiler::arrayElementConst(std::string const &name, int index) {
  assert(_currentFunction != nullptr);
  return arrayElementConst(local(name), index);
}

Slot Compiler::arrayElementConst(Slot const &slot, int index) {
  assert(types::isArray(slot.type));  
  assert(index < slot.type->length());

  // Create a new slot that represents the element
  return Slot {
    .name = std::string("__elem_") + slot.name + "_" + std::to_string(index),
    .type = slot.type->elementType(),
    .kind = Slot::ArrayElement,
    .offset = slot.offset + (index * slot.type->elementType()->size())
  };
}


void Compiler::callFunction(std::string const &functionName,
			    std::string const &nextBlockName,
			    values::Var const &returnVar) {
  callFunction(functionName, nextBlockName, {}, returnVar);
}

void Compiler::callFunction(std::string const &functionName,
			    std::string const &nextBlockName,
			    std::vector<values::Value> const &args,
			    values::Var const &returnVar) {
  
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
      .returnVar = (returnVar != nullptr) ? returnVar->varName() : "",
      .nextBlockName = nextBlockName,
    });

  deferFunctionCallTypeCheck(_currentFunction->name, functionName, args);
  copyArgsToNextFrame(functionName, args);
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


void Compiler::returnFromFunction() {
  syncLocalToGlobal();
  popFrame();
  _nextBlockIsSet = true;
}

void Compiler::returnFromFunction(Slot const &slot) {
  assert(_currentBlock != nullptr);
  assert(slot.type == _currentFunction->sig.returnType); // TODO: convert API-level asserts exceptions/errors

  // Copy the variable into the return-slot. TODO: non-globals can be moved rather than copied
  Slot returnSlot = {
    .name = "__return_slot",
    .type = slot.type,
    .kind = Slot::Dummy,
    .offset = FrameLayout::ReturnValueStart,
    .scope = nullptr
  };

  assign(returnSlot, slot);
  returnFromFunction();
}

void Compiler::returnFromFunction(values::Value const &value) {
  assert(_currentBlock != nullptr);
  assert(value->type(_ts) == _currentFunction->sig.returnType); // TODO: convert API-level asserts exceptions/errors

  Slot returnSlot = {
    .name = "__return_slot",
    .type = value->type(_ts),
    .kind = Slot::Dummy,
    .offset = FrameLayout::ReturnValueStart,
    .scope = nullptr
  };

  assign(returnSlot, value);
  returnFromFunction();
}

void Compiler::returnFromFunction(values::Var const &var) {
  return returnFromFunction(local(var->varName()));
}


void Compiler::assign(Slot const &dest, Slot const &src) {
  assert(dest.type == src.type);
  types::TypeHandle const type = dest.type;
  
  // Copy src into dest
  for (int i = 0; i != type->size(); ++i) {
    moveTo(dest + i, MacroCell::Value0);
    zeroCell();
    moveTo(src + i, MacroCell::Value0);
    copyField(dest + i, MacroCell::Value0);
    if (type->usesValue1()) {
      moveTo(dest + i, MacroCell::Value1);
      zeroCell();
      moveTo(src + i, MacroCell::Value1);
      copyField(dest + i, MacroCell::Value1);
    }
  }
}

void Compiler::assign(Slot const &slot, values::Value const &value) {

  if (value->type(_ts) == nullptr) {
    // Variable, assign from slot.
    return assign(slot, value->varName());
  }

  // Constant -> construct in slot
  assert(slot.type == value->type(_ts));
  
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
    else if (types::isArray(slot.type)) {
      for (int i = 0; i != val->type(_ts)->length(); ++i) {
	self(self, arrayElementConst(slot, i), val->element(i));
      }
    }
    else {
      assert(false && "not implemented");
    }
  };

  constructInSlot(constructInSlot, slot, value);
}

void Compiler::assign(Slot const &slot, values::Var const &var) {
  assign(slot, local(var->varName()));
}

void Compiler::assign(values::Var const &var, values::Value const &value) {
  assign(local(var->varName()), value);
}


void Compiler::assign(values::Var const &destVar, values::Var const &srcVar) {
  assign(local(destVar->varName()), local(srcVar->varName()));
}

void Compiler::writeOut(values::Value const &value) {
  if (value->type(_ts) == types::null) return writeOut(value->varName());

  Slot const tmp = getTemp(value);
  writeOut(tmp);
}

void Compiler::writeOut(values::Var const &var) {
  writeOut(local(var->varName()));
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

