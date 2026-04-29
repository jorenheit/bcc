// Tests a function pointer whose target itself calls another function pointer.
// main calls outer through a pointer; outer receives a callback pointer and calls it.
// The sequence is repeated with a different callback.
// Expected: XY

TEST_BEGIN

auto voidT = ts::void_t();
auto callbackType = ts::function().ret(voidT).done();
auto callbackPtr = ts::function_pointer(callbackType);
auto outerType = ts::function().ret(voidT).param(callbackPtr).done();
auto outerPtr = ts::function_pointer(outerType);

c.function("main").begin(); {
  c.declareLocal("outer", outerPtr);
  c.declareLocal("callback", callbackPtr);

  c.block("entry").begin(); {
    c.assign("outer", literal::function_pointer(outerType, "callCallback"));
    c.assign("callback", literal::function_pointer(callbackType, "printX"));
    c.callFunctionPointer("outer", "second").arg("callback").done();
  } c.endBlock();

  c.block("second").begin(); {
    c.assign("callback", literal::function_pointer(callbackType, "printY"));
    c.callFunctionPointer("outer", "end").arg("callback").done();
  } c.endBlock();

  c.block("end").begin(); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("callCallback").param("callback", callbackPtr).ret(voidT).begin(); {
  c.block("entry").begin(); {
    c.callFunctionPointer("callback", "done").done();
  } c.endBlock();

  c.block("done").begin(); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("printX").begin(); {
  c.block("entry").begin(); {
    c.writeOut(literal::i8('X'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("printY").begin(); {
  c.block("entry").begin(); {
    c.writeOut(literal::i8('Y'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
