// Tests a function pointer call whose callee returns another function pointer.
// The returned pointer is then called, then the selector pointer is overwritten
// and the sequence is repeated. This stresses indirect call return transport
// and overwriting pointer locals after indirect calls.
// Expected: SATB

TEST_BEGIN

auto voidT = TypeSystem::voidT();
auto actionType = TypeSystem::function(voidT);
auto actionPtr = TypeSystem::function_pointer(actionType);
auto selectorType = TypeSystem::function(actionPtr);
auto selectorPtr = TypeSystem::function_pointer(selectorType);

c.beginFunction("main"); {
  c.declareLocal("selector", selectorPtr);
  c.declareLocal("action", actionPtr);

  c.beginBlock("entry"); {
    c.assign("selector", values::function_pointer(selectorType, "selectA"));
    c.callFunctionPointer("selector", "callA", "action")();
  } c.endBlock();

  c.beginBlock("callA"); {
    c.callFunctionPointer("action", "selectB")();
  } c.endBlock();

  c.beginBlock("selectB"); {
    c.assign("selector", values::function_pointer(selectorType, "selectB"));
    c.callFunctionPointer("selector", "callB", "action")();
  } c.endBlock();

  c.beginBlock("callB"); {
    c.callFunctionPointer("action", "end")();
  } c.endBlock();

  c.beginBlock("end"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("selectA", selectorType); {
  c.beginBlock("entry"); {
    c.writeOut(values::i8('S'));
    c.returnFromFunction(values::function_pointer(actionType, "printA"));
  } c.endBlock();
} c.endFunction();

c.beginFunction("selectB", selectorType); {
  c.beginBlock("entry"); {
    c.writeOut(values::i8('T'));
    c.returnFromFunction(values::function_pointer(actionType, "printB"));
  } c.endBlock();
} c.endFunction();

c.beginFunction("printA"); {
  c.beginBlock("entry"); {
    c.writeOut(values::i8('A'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("printB"); {
  c.beginBlock("entry"); {
    c.writeOut(values::i8('B'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
