// Tests a function pointer whose target itself calls another function pointer.
// main calls outer through a pointer; outer receives a callback pointer and calls it.
// The sequence is repeated with a different callback.
// Expected: XY

TEST_BEGIN

auto voidT = ts::voidT();
auto callbackType = ts::function(voidT)();
auto callbackPtr = ts::function_pointer(callbackType);
auto outerType = ts::function(voidT)(callbackPtr);
auto outerPtr = ts::function_pointer(outerType);

c.beginFunction("main"); {
  c.declareLocal("outer", outerPtr);
  c.declareLocal("callback", callbackPtr);

  c.beginBlock("entry"); {
    c.assign("outer", literal::function_pointer(outerType, "callCallback"));
    c.assign("callback", literal::function_pointer(callbackType, "printX"));
    c.callFunctionPointer("outer", "second")("callback");
  } c.endBlock();

  c.beginBlock("second"); {
    c.assign("callback", literal::function_pointer(callbackType, "printY"));
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
    c.writeOut(literal::i8('X'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("printY"); {
  c.beginBlock("entry"); {
    c.writeOut(literal::i8('Y'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
