// Tests a function pointer passed through several function frames before it is called.
// main -> pass1 -> pass2 -> pass3, and only pass3 performs the indirect call.
// Expected: Z

TEST_BEGIN

auto voidT = ts::void_t();
auto actionType = ts::function().ret(voidT).done();
auto actionPtr = ts::function_pointer(actionType);

c.function("main").begin(); {
  c.declareLocal("fp", actionPtr);

  c.block("entry").begin(); {
    c.assign("fp", literal::function_pointer(actionType, "printZ"));
    c.callFunction("pass1", "end").arg("fp").done();
  } c.endBlock();

  c.block("end").begin(); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("pass1").param("fp", actionPtr).ret(voidT).begin(); {
  c.block("entry").begin(); {
    c.callFunction("pass2", "done").arg("fp").done();
  } c.endBlock();

  c.block("done").begin(); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("pass2").param("fp", actionPtr).ret(voidT).begin(); {
  c.block("entry").begin(); {
    c.callFunction("pass3", "done").arg("fp").done();
  } c.endBlock();

  c.block("done").begin(); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("pass3").param("fp", actionPtr).ret(voidT).begin(); {
  c.block("entry").begin(); {
    c.callFunctionPointer("fp", "done").done();
  } c.endBlock();

  c.block("done").begin(); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("printZ").begin(); {
  c.block("entry").begin(); {
    c.writeOut(literal::i8('Z'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
