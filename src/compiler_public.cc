#include "compiler.ih"

void Compiler::setEntryPoint(std::string functionName, API_FUNC) {
  API_FUNC_BEGIN("setEntryPoint");
  API_REQUIRE_OUTSIDE_PROGRAM_BLOCK();
  
  _program.entryFunctionName = std::move(functionName);
}

void Compiler::begin(API_FUNC) {
  API_FUNC_BEGIN("begin");
  API_CHECK_EXPECTED();
  API_REQUIRE(not _program.entryFunctionName.empty(), "no entry point set; call 'setEntryPoint' first.");
  API_REQUIRE_OUTSIDE_PROGRAM_BLOCK();

  _state.begun = true;

  // Globals should start at same frame offset as locals for consistency -> pad with raw
  declareGlobal("__pad__", TypeSystem::raw(FrameLayout::ReturnValueStart)); 
}

void Compiler::end(API_FUNC) {
  API_FUNC_BEGIN("end");
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_PROGRAM_BLOCK();
  API_REQUIRE_OUTSIDE_FUNCTION_BLOCK();
      
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
}

void Compiler::beginFunction(std::string const &name, types::TypeHandle type, std::vector<std::string> const &params, API_FUNC) {
  API_FUNC_BEGIN("beginFunction");
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

void Compiler::beginFunction(std::string const &name, API_FUNC) {
  beginFunction(name, TypeSystem::function(TypeSystem::voidT()), std::vector<std::string>{});
}

void Compiler::beginFunction(std::string const &name, types::TypeHandle funcType, API_FUNC) {
  beginFunction(name, funcType, std::vector<std::string>{});
}

void Compiler::endFunction(API_FUNC) {
  API_FUNC_BEGIN("endFunction");
  API_CHECK_EXPECTED();  
  API_REQUIRE_INSIDE_FUNCTION_BLOCK();
  API_REQUIRE_NO_SCOPE();
  
  _currentFunction = nullptr;
}

void Compiler::beginScope(API_FUNC) {
  API_FUNC_BEGIN("beginScope");
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_FUNCTION_BLOCK();
  API_REQUIRE_OUTSIDE_CODE_BLOCK();
  
  _currentScope = &_currentFunction->createScope(_currentScope);
}

void Compiler::endScope(API_FUNC) {
  API_FUNC_BEGIN("endScope");
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_FUNCTION_BLOCK();
  API_REQUIRE_OUTSIDE_CODE_BLOCK();
  
  freeScope(_currentScope);
  _currentScope = _currentScope->parent;
}


void Compiler::beginBlock(std::string name, API_FUNC) {
  API_FUNC_BEGIN("beginBlock");
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

void Compiler::endBlock(API_FUNC) {
  API_FUNC_BEGIN("endBlock");
  API_CHECK_EXPECTED_STRICT();
  API_REQUIRE_INSIDE_CODE_BLOCK();

  blockClose();
  freeTemps();
  _currentBlock = nullptr;
}

void Compiler::setNextBlock(int index, API_FUNC) {
  API_FUNC_BEGIN("setNextBlock");
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();

  setNextBlockImpl(index);

  API_EXPECT_NEXT("endBlock");
}


void Compiler::setNextBlockImpl(int index) {
  pushPtr();
  moveTo(FrameLayout::TargetBlock, MacroCell::Value0);
  setToValue16(index, Cell{FrameLayout::TargetBlock, MacroCell::Value1});
  popPtr();
  _nextBlockIsSet = true;
}

void Compiler::setNextBlock(std::string const &f, std::string const &b, API_FUNC) {
  API_FUNC_BEGIN("setNextBlock");
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();

  setNextBlockImpl(f, b);

  API_EXPECT_NEXT("endBlock");  
}

void Compiler::setNextBlock(std::string const &b, API_FUNC) {
  API_FUNC_BEGIN("setNextBlock");
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();

  setNextBlockImpl(_currentFunction->name, b);

  API_EXPECT_NEXT("endBlock");  
}

void Compiler::setNextBlockImpl(std::string const &f, std::string const &b) {
  // It is possible that the function or block name has not been
  // defined yet. So we need to check for this first.
  if (_program.isFunctionDefined(f)) {
    Function const &func = _program.function(f);
    if (func.isBlockDefined(b)) {
      // both defined -> use global block index
      return setNextBlockImpl(func.block(b).globalBlockIndex);
    }
  }

  pushPtr();
  
  // TODO: proper error for undefined block names
  
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
    


Slot Compiler::declareGlobal(std::string const &name, types::TypeHandle type, API_FUNC) {
  API_FUNC_BEGIN("declareGlobal");
  API_CHECK_EXPECTED();
  API_REQUIRE_DECLARE_GLOBAL_ALLOWED();
  API_REQUIRE_INSIDE_PROGRAM_BLOCK();
  API_REQUIRE_OUTSIDE_FUNCTION_BLOCK();
  API_REQUIRE_GLOBAL_NAME_AVAILABLE(name);

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


Slot Compiler::declareLocal(std::string const& name, types::TypeHandle type, API_FUNC) {
  API_FUNC_BEGIN("declareLocal");
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_FUNCTION_BLOCK();
  API_REQUIRE_OUTSIDE_CODE_BLOCK();
  API_REQUIRE_NOT_IN_CURRENT_SCOPE(name);

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


void Compiler::referGlobals(std::vector<std::string> const &names, API_FUNC) {
  API_FUNC_BEGIN("referGlobals");
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_FUNCTION_BLOCK();
  API_REQUIRE_OUTSIDE_CODE_BLOCK();
  
  beginBlock(std::string("__prologue_") + _currentFunction->name); {

    std::unordered_set<std::string> declared;
    for (std::string const &name: names) {
      API_REQUIRE_IS_GLOBAL(name);

      auto [_, unique] = declared.insert(name);
      API_REQUIRE(unique, "multiple references to ", name, ".");
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

  return Slot::invalid();
}

types::TypeHandle Compiler::defineStruct(std::string const& name, StructFields const &fields, API_FUNC) {
  API_FUNC_BEGIN("defineStruct");
  API_REQUIRE_INSIDE_PROGRAM_BLOCK();
  API_REQUIRE_OUTSIDE_FUNCTION_BLOCK();
    
  types::TypeHandle sType = TypeSystem::defineStruct(name, std::move(fields));
  API_REQUIRE(sType != nullptr, "conficting struct declaration: '", name, "' previously defined.");
  return sType;
}

ExpressionResult Compiler::structFieldImpl(ExpressionResult const &obj, int fieldIndex, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_STRUCT(obj);
  API_REQUIRE_FIELD_INDEX_IN_BOUNDS(obj, fieldIndex);
  
  auto structType = types::cast<types::StructType>(obj.type());  
  return structFieldImpl(obj, structType->fieldName(fieldIndex), API_FWD);
}

ExpressionResult Compiler::structFieldImpl(ExpressionResult const &obj, std::string const &fieldName, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_STRUCT(obj);
  API_REQUIRE_IS_FIELD(obj, fieldName);

  if (obj.isLiteral()) {
    auto structVal = values::cast<types::StructType>(obj.literal());
    return ExpressionResult{structVal->field(fieldName)};
  }
  
  return ExpressionResult{proxy::structField(obj.slot(), fieldName)};
}

ExpressionResult Compiler::arrayElementImpl(ExpressionResult const &arr, int index, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_ARRAY_OR_STRING(arr);
  API_REQUIRE_INDEX_IN_BOUNDS(arr, index);

  if (arr.isLiteral()) {
    auto arrVal = values::cast<types::ArrayLike>(arr.literal());
    return ExpressionResult{arrVal->element(index)};
  }
  
  return ExpressionResult{proxy::arrayElement(arr.slot(), index)};  
}  

ExpressionResult Compiler::arrayElementImpl(ExpressionResult const &arr, ExpressionResult const &index, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_ARRAY_OR_STRING(arr);
  API_REQUIRE_IS_INTEGER(index);

  if (index.isLiteral()) {
    int const i = values::cast<types::IntegerType>(index.literal())->value();
    return arrayElementImpl(arr, i, API_FWD);
  }

  if (arr.isLiteral()) {
    Slot const tmp = getTemp(arr.type());
    assignSlot(tmp, arr.literal());
    return ExpressionResult{proxy::arrayElement(tmp, index.slot())};
  }
  
  return ExpressionResult{proxy::arrayElement(arr.slot(), index.slot())};
}

ExpressionResult Compiler::dereferencePointerImpl(ExpressionResult const &ptr, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_POINTER(ptr);
  assert(not ptr.isLiteral());
  
  return ExpressionResult{proxy::dereferencedPointer(ptr.slot())};
}

void Compiler::callFunction(std::string const& functionName, std::string const& nextBlockName, ArgList const &args, API_FUNC) {
  API_FUNC_BEGIN("callFunction");
  callFunctionImpl(functionName, nextBlockName, {}, args, API_FWD);
}

void Compiler::callFunction(std::string const& functionName, std::string const& nextBlockName, API_FUNC) {
  API_FUNC_BEGIN("callFunction");
  callFunctionImpl(functionName, nextBlockName, {}, {}, API_FWD);
}

void Compiler::callFunctionImpl(std::string const& functionName, std::string const& nextBlockName,
				std::optional<ExpressionResult> const &returnSlot, ArgList const &args, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();

  syncLocalToGlobal();

  std::string const metaBlockName = std::string("__ret_meta_") + _currentFunction->name + "_" + std::to_string(_metaBlocks.size());

  setNextBlockImpl(_currentFunction->name, metaBlockName);
  _metaBlocks.push_back(MetaBlock{
      .name = metaBlockName,
      .caller = _currentFunction->name,
      .callee = functionName,
      .returnSlot = returnSlot ? std::optional<SlotProxy>(returnSlot->slot()) : std::nullopt,
      .nextBlockName = nextBlockName,
    });

  deferFunctionCallTypeCheck(_currentFunction->name, functionName, args, API_FWD);
  initializeArguments(functionName, args, API_FWD);
  pushFrame();

  setNextBlockImpl(functionName, "");
  API_EXPECT_NEXT("endBlock");
}


void Compiler::abortProgram(API_FUNC) {
  API_FUNC_BEGIN("abortProgram");
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();

  moveTo(FrameLayout::RunState, MacroCell::Value0);
  zeroCell();

  // Sync and pop
  popFrame();
  _nextBlockIsSet = true; // TODO: shouldn't need this flag when macro's have been implemented

  API_EXPECT_NEXT("endBlock");
}

void Compiler::returnFromFunction(API_FUNC) {
  API_FUNC_BEGIN("returnFromFunction");
  returnFromFunctionImpl({}, API_FWD);
}

void Compiler::returnFromFunctionImpl(std::optional<ExpressionResult> const &ret, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  
  if (ret) {
    API_REQUIRE_ASSIGNABLE(ret->type(), _currentFunction->type->returnType());

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
  
  syncLocalToGlobal();
  popFrame();
  _nextBlockIsSet = true;
  API_EXPECT_NEXT("endBlock");
}


void Compiler::branchIfImpl(ExpressionResult const &obj, std::string const &trueLabel,
			    std::string const &falseLabel, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_IS_INTEGER(obj);

  if (obj.hasSlot()) {
    branchIfSlot(obj.slot()->materialize(*this), trueLabel, falseLabel);
  } else {  
    bool const value = values::cast<types::IntegerType>(obj.literal())->value();
    setNextBlockImpl(_currentFunction->name, value ? trueLabel : falseLabel);
  }
  
  API_EXPECT_NEXT("endBlock");
}

void Compiler::branchIfSlot(Slot const &slot, std::string const &trueLabel, std::string const &falseLabel) {

  pushPtr();

  Slot const tmp = getTemp(TypeSystem::i8());
    
  moveTo(slot);
  if (slot.type->usesValue1()) {
    orConstructive(Cell{tmp, MacroCell::Value0},
		   Cell{slot, MacroCell::Value1},
		   Temps<2>::select(tmp, MacroCell::Scratch0,
				    tmp, MacroCell::Scratch1));
  }
  else {
    copyField(Cell{tmp, MacroCell::Value0},
	      Temps<1>::select(tmp, MacroCell::Scratch0));
  }

  moveTo(tmp);
  switchField(MacroCell::Flag);
  setToValue(1);
  switchField(MacroCell::Value0);
  loopOpen(); {
    zeroCell();
    setNextBlockImpl(_currentFunction->name, trueLabel);
    switchField(MacroCell::Flag);
    setToValue(0);
    switchField(MacroCell::Value0);	
  }; loopClose();

  switchField(MacroCell::Flag);
  loopOpen(); {
    zeroCell();
    setNextBlockImpl(_currentFunction->name, falseLabel);
  } loopClose();

  popPtr();
}

ExpressionResult Compiler::addressOfImpl(ExpressionResult const &obj, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  assert(not obj.isLiteral());
  
  if (obj.slot()->direct()) {
    Slot const slot = obj.slot()->materialize(*this);
    API_REQUIRE(slot.kind != Slot::Temp, "Cannot take the address of a temporary value.");
  }
  
  return ExpressionResult(obj.slot()->addressOf(*this));
}


Slot Compiler::addressOfSlot(Slot const &slot) {
  assert(slot.kind != Slot::Temp && "taking address of temp");

  types::TypeHandle const pointeeType = slot.type;
  types::TypeHandle const pointerType = TypeSystem::pointer(pointeeType);

  int offset = slot.offset;
  bool localPointer = true;
  if (slot.kind == Slot::Kind::GlobalReference) {
    std::string const globalName = slot.name.substr(std::string("__g_").size());
    assert(_program.isGlobal(globalName));
    Slot const globalSlot = _program.globalSlot(globalName);
    assert(globalSlot.type == pointeeType);
    offset = globalSlot.offset;
    localPointer = false;
    _aliasedGlobals.insert(globalName);
  }

  // Set frame-depth to 0 for a local pointer, FrameID for a global pointer
  Slot const ptrSlot = getTemp(pointerType);
  if (localPointer) {
    moveTo(ptrSlot + RuntimePointer::FrameDepth, MacroCell::Value0);
    zeroCell();
  } else {
    moveTo(0, MacroCell::FrameMarker);
    copyField(Cell{ptrSlot + RuntimePointer::FrameDepth, MacroCell::Value0},
	      Temps<1>::select(ptrSlot + RuntimePointer::FrameDepth, MacroCell::Scratch0));
  }

  // Construct offset in second cell
  moveTo(ptrSlot + RuntimePointer::Offset, MacroCell::Value0);
  setToValue(offset & 0xff);
  moveTo(ptrSlot + RuntimePointer::Offset, MacroCell::Value1);
  setToValue((offset >> 8) & 0xff);

  return ptrSlot;
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
	   Temps<3>::select(scaledIndexSlot, MacroCell::Scratch0,
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
	   Temps<3>::select(scaledIndexSlot, MacroCell::Scratch0,
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
	      Temps<1>::select(arrSlot + i, MacroCell::Scratch0));
    if (elementType->usesValue1()) {
      moveTo(srcSlot + i, MacroCell::Value1);
      copyField(Cell{arrSlot + i, MacroCell::Payload1},
		Temps<1>::select(arrSlot + i, MacroCell::Scratch0));
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
  assert(dest.size() >= src.size());

  pushPtr();
  // Copy src into slot
  for (int i = 0; i != dest.size(); ++i) {
    moveTo(src + i, MacroCell::Value0);
    copyField(Cell{dest + i, MacroCell::Value0},
	      Temps<1>::select(dest + i, MacroCell::Scratch0));
    moveTo(src + i, MacroCell::Value1);
    if (dest.type->usesValue1()) {
      copyField(Cell{dest + i, MacroCell::Value1},
		Temps<1>::select(dest + i, MacroCell::Scratch0));
    }
    else {
      setToValue(0);
    }
  }
  popPtr();
}

void Compiler::assignSlot(Slot const &slot, values::Literal const &val) {
  pushPtr();
  if (types::isInteger(slot.type)) {
    int const x = values::cast<types::IntegerType>(val)->value();
    moveTo(slot, MacroCell::Value0);
    setToValue(x & 0xff);
    moveTo(slot, MacroCell::Value1);    
    if (slot.type->usesValue1()) {
      setToValue((x >> 8) & 0xff);
    }
    else {
      setToValue(0);      
    }
  }
  else if (types::isArray(slot.type) || types::isString(slot.type)) {
    // recursive call for each element
    for (int i = 0; i != types::cast<types::ArrayLike>(val->type())->length(); ++i) {
      ExpressionResult elem = arrayElement(slot, i);
      assert(elem.hasSlot());
      elem.slot()->write(*this, values::cast<types::ArrayLike>(val)->element(i));
    }
  }
  else if (types::isStruct(slot.type)) {
    // recursive call for each field	
    for (int i = 0; i != types::cast<types::StructType>(val->type())->fieldCount(); ++i) {
      ExpressionResult field = structField(slot, i);
      assert(field.hasSlot());
      field.slot()->write(*this, values::cast<types::StructType>(val)->field(i));
    }
  }
  else {
    assert(false && "not implemented");
  }
  popPtr();
}


ExpressionResult Compiler::assignImpl(ExpressionResult const &lhs, ExpressionResult const &rhs, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_ASSIGNABLE(lhs.type(), rhs.type());

  if (rhs.hasSlot()) {
    lhs.slot()->write(*this, rhs.slot());
  }
  else {
    lhs.slot()->write(*this, rhs.literal());
  }

  return lhs;
}

void Compiler::writeOutImpl(ExpressionResult const &rhs, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();

  pushPtr();

  Slot const slot = rhs.hasSlot()
    ? rhs.slot()->materialize(*this)
    : getTemp(rhs.literal());

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
  copyField(frameDepthPayload, Temps<1>::select(frameDepthPayload, MacroCell::Scratch0));

  moveTo(offsetLow);
  copyField(offsetLowPayload,  Temps<1>::select(offsetLowPayload, MacroCell::Scratch0));

  moveTo(offsetHigh);
  copyField(offsetHighPayload, Temps<1>::select(offsetHighPayload, MacroCell::Scratch0));

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
	      Temps<1>::select(srcSlot + i, MacroCell::Scratch0));
    if (srcSlot.type->usesValue1()) {
      moveTo(srcSlot + i, MacroCell::Value1);
      copyField(Cell{srcSlot + i, MacroCell::Payload1},
		Temps<1>::select(srcSlot + i, MacroCell::Scratch0));
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

// TODO: factor common code with write
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
  copyField(frameDepthPayload, Temps<1>::select(frameDepthPayload, MacroCell::Scratch0));

  moveTo(offsetLow);
  copyField(offsetLowPayload,  Temps<1>::select(offsetLowPayload, MacroCell::Scratch0));

  moveTo(offsetHigh);
  copyField(offsetHighPayload, Temps<1>::select(offsetHighPayload, MacroCell::Scratch0));

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
    copyField(Cell{i, MacroCell::Payload0}, Temps<1>::select(i, MacroCell::Scratch0));
    if (derefSlot.type->usesValue1()) {
      moveTo(i, MacroCell::Value1);    
      copyField(Cell{i, MacroCell::Payload1}, Temps<1>::select(i, MacroCell::Scratch0));
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


