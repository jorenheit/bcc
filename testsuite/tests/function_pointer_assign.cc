// Tests assigning function-pointer literals to a local and then overwriting that local.
// This should catch stale function-pointer target state after reassignment.
// Expected: AB

TEST_BEGIN

auto voidT = ts::void_t();
auto fnType = ts::function().ret(voidT).done();
auto fnPtr = ts::function_pointer(fnType);

c.beginFunction("main"); {
  c.declareLocal("fptr", fnPtr);

  c.beginBlock("entry"); {
    c.assign("fptr", literal::function_pointer(fnType, "printA"));
    c.callFunctionPointer("fptr", "second").done();
  } c.endBlock();

  c.beginBlock("second"); {
    c.assign("fptr", literal::function_pointer(fnType, "printB"));
    c.callFunctionPointer("fptr", "end").done();
  } c.endBlock();

  c.beginBlock("end"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("printA"); {
  c.beginBlock("entry"); {
    c.writeOut(literal::i8('A'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("printB"); {
  c.beginBlock("entry"); {
    c.writeOut(literal::i8('B'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
