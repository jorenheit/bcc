// Tests a function pointer whose target itself calls another function pointer.
// main calls outer through a pointer; outer receives a callback pointer and calls it.
// The sequence is repeated with a different callback.
// Expected: XY

TEST_BEGIN

auto voidT = TypeSystem::voidT();
auto callbackType = TypeSystem::function(voidT);
auto callbackPtr = TypeSystem::function_pointer(callbackType);
auto outerType = TypeSystem::function(voidT, callbackPtr);
auto outerPtr = TypeSystem::function_pointer(outerType);

c.beginFunction("main"); {
  c.declareLocal("outer", outerPtr);
  c.declareLocal("callback", callbackPtr);

  c.beginBlock("entry"); {
    c.assign("outer", values::function_pointer(outerType, "callCallback"));
    c.assign("callback", values::function_pointer(callbackType, "printX"));
    c.callFunctionPointer("outer", "second")("callback");
  } c.endBlock();

  c.beginBlock("second"); {
    c.assign("callback", values::function_pointer(callbackType, "printY"));
    c.callFunctionPointer("outer", "end")("callback");
  } c.endBlock();

  c.beginBlock("end"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("callCallback", outerType, {"callback"}); {
  c.beginBlock("entry"); {
    c.callFunctionPointer("callback", "done")();
  } c.endBlock();

  c.beginBlock("done"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("printX"); {
  c.beginBlock("entry"); {
    c.writeOut(values::i8('X'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("printY"); {
  c.beginBlock("entry"); {
    c.writeOut(values::i8('Y'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
