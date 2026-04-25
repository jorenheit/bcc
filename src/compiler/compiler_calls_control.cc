#include "compiler.ih"

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

void Compiler::deferFunctionCallTypeCheck(std::string const &caller,
					  std::string const &callee,
					  std::vector<ExpressionResult> const &args, API_CTX) {
  auto const getHandles = [&](){
    std::vector<types::TypeHandle> result;
    for (auto const &arg: args) {
      result.push_back(arg.type());
    }
    return result;
  };

  _deferredFunctionCallTypeChecks.emplace_back(FunctionCall{
      .API_CTX_NAME = API_FWD,
      .caller = caller,
      .callee = callee,
      .args = getHandles()
    });
}

void Compiler::functionCallTypeChecks() {
  assert(_currentFunction == nullptr);

  for (auto const &[API_CTX_NAME, caller, callee, args]: _deferredFunctionCallTypeChecks) {
    auto const &paramTypes = _program.function(callee).type->paramTypes();
    API_REQUIRE(paramTypes.size() == args.size(),
		"invalid number of arguments in function-call to '", callee, "': "
		"expected ", paramTypes.size(), ", got ", args.size(), ".");
    for (size_t i = 0; i != args.size(); ++i) {
      API_REQUIRE_ASSIGNABLE(paramTypes[i], args[i]);
    }
  }

  _deferredFunctionCallTypeChecks.clear();
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

