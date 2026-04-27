#include "builder.ih"

void Builder::setEntryPoint(std::string functionName, API_FUNC) {
  API_FUNC_BEGIN();
  API_REQUIRE_OUTSIDE_PROGRAM_BLOCK();
  
  _program.entryFunctionName = std::move(functionName);
}

void Builder::begin(API_FUNC) {
  API_FUNC_BEGIN();
  API_CHECK_EXPECTED();
  API_REQUIRE(not _program.entryFunctionName.empty(), "no entry point set; call 'setEntryPoint' first.");
  API_REQUIRE_OUTSIDE_PROGRAM_BLOCK();

  _state.begun = true;

  // Globals should start at same frame offset as locals for consistency -> pad with raw
  declareGlobal("__pad__", TypeSystem::raw(FrameLayout::ReturnValueStart)); 
}

void Builder::end(API_FUNC) {
  API_FUNC_BEGIN();
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_PROGRAM_BLOCK();
  API_REQUIRE_OUTSIDE_FUNCTION_BLOCK();
  API_REQUIRE(_program.functions.size() > 0, "a program should contain at least one function.");
      
  deferredFunctionCallTypeChecks();
  deferredBlockNameChecks();
  
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

  setNextBlockImpl(_program.entryFunctionName, "");
  moveTo(FrameLayout::RunState, MacroCell::Value0);
  setToValue(1);
  loopOpen("main loop");
  moveToOrigin();
  
  // Also generate the hatstrap code. All this needs to do is move the pointer to
  // the run-cell and close the loop.

  setTargetSequence(&_program.hatstrap);
  moveTo(FrameLayout::RunState, MacroCell::Value0);
  loopClose("main loop");

  _state.begun = false;
}

void Builder::beginFunction(std::string const &name, types::TypeHandle type, std::vector<std::string> const &params, API_FUNC) {
  API_FUNC_BEGIN();
  API_CHECK_EXPECTED();  
  API_REQUIRE_INSIDE_PROGRAM_BLOCK();
  API_REQUIRE_OUTSIDE_FUNCTION_BLOCK();
  API_REQUIRE_IS_FUNCTION(type);

  auto fType = types::cast<types::FunctionType>(type);
  API_REQUIRE_PARAM_COUNT_MATCHES_FUNCTION(fType, params);
  
  _state.allowGlobalDefinitions = false;
  _currentFunction = &_program.createFunction(name, fType);  

  std::unordered_set<std::string> paramSet;
  for (size_t i = 0; i != params.size(); ++i) {
    std::string const &name = params[i];
    auto [_, unique] = paramSet.insert(name);
    API_REQUIRE(unique, "parameter name '", name, "' used more than once.");
    declareLocal(name, fType->paramTypes()[i]);
  }
}

void Builder::beginFunction(std::string const &name, API_FUNC) {
  beginFunction(name, TypeSystem::function(TypeSystem::voidT()), std::vector<std::string>{});
}

void Builder::beginFunction(std::string const &name, types::TypeHandle funcType, API_FUNC) {
  beginFunction(name, funcType, std::vector<std::string>{});
}

void Builder::endFunction(API_FUNC) {
  API_FUNC_BEGIN();
  API_CHECK_EXPECTED();  
  API_REQUIRE_INSIDE_FUNCTION_BLOCK();
  API_REQUIRE_NO_SCOPE();
  API_REQUIRE(_currentFunction->blocks.size() > 0, "a function should contain at least 1 code-block.");
  
  _currentFunction = nullptr;
}

void Builder::beginScope(API_FUNC) {
  API_FUNC_BEGIN();
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_FUNCTION_BLOCK();
  API_REQUIRE_OUTSIDE_CODE_BLOCK();
  
  _currentScope = &_currentFunction->createScope(_currentScope);
}

void Builder::endScope(API_FUNC) {
  API_FUNC_BEGIN();
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_FUNCTION_BLOCK();
  API_REQUIRE_OUTSIDE_CODE_BLOCK();
  
  freeScope(_currentScope);
  _currentScope = _currentScope->parent;
}


void Builder::beginBlock(std::string name, API_FUNC) {
  API_FUNC_BEGIN();
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_FUNCTION_BLOCK();
  API_REQUIRE_OUTSIDE_CODE_BLOCK();
  
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

void Builder::endBlock(API_FUNC) {
  API_FUNC_BEGIN();
  API_CHECK_EXPECTED_STRICT();
  API_REQUIRE_INSIDE_CODE_BLOCK();

  blockClose();
  freeTemps();
  _currentBlock = nullptr;
}

void Builder::setNextBlock(std::string const &f, std::string const &b, API_FUNC) {
  API_FUNC_BEGIN();
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();

  setNextBlockImpl(f, b);
  deferBlockNameCheck(f, b, API_FWD);

  API_EXPECT_NEXT("endBlock");  
}

void Builder::setNextBlock(std::string const &b, API_FUNC) {
  API_FUNC_BEGIN();
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();

  setNextBlockImpl(_currentFunction->name, b);
  deferBlockNameCheck(_currentFunction->name, b, API_FWD);

  API_EXPECT_NEXT("endBlock");  
}


void Builder::setNextBlockImpl(std::string const &f, std::string const &b) {

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

void Builder::setNextBlockImpl(int index) {
  pushPtr();
  moveTo(FrameLayout::TargetBlock, MacroCell::Value0);
  setToValue16(index, Cell{FrameLayout::TargetBlock, MacroCell::Value1});
  popPtr();
  _nextBlockIsSet = true;
}

void Builder::setNextBlockImpl(Expression const &obj) {
  assert(types::isFunctionPointer(obj.type()));
  
  Slot const targetSlot {
    .name = "target_block",
    .type = obj.type(),
    .kind = Slot::Dummy,
    .offset = FrameLayout::TargetBlock
  };
  
  if (obj.hasSlot()) {
    Slot const ptrSlot = obj.slot()->materialize(*this);
    assignSlot(targetSlot, ptrSlot);
  } else {
    assignSlot(targetSlot, obj.literal());
  }

  pushPtr();
  moveTo(FrameLayout::TargetBlock);
  emit<primitive::Out>();
  popPtr();
  
  _nextBlockIsSet = true;  
}


