// Tests calling function pointers with i8 arguments.
// The two targets both receive an argument, but add their own prefix so dispatch and argument transport are both checked.
// Expected: LARB

TEST_BEGIN

auto i8 = ts::i8();
auto voidT = ts::void_t();
auto fnType = ts::function().ret(voidT).param(i8).done();
auto fnPtr = ts::function_pointer(fnType);

c.beginFunction("main"); {
  c.declareLocal("fptr", fnPtr);
  c.declareLocal("v", i8);

  c.beginBlock("entry"); {
    c.assign("v", literal::i8('A'));
    c.assign("fptr", literal::function_pointer(fnType, "leftEcho"));
    c.callFunctionPointer("fptr", "second").arg("v").done();
  } c.endBlock();

  c.beginBlock("second"); {
    c.assign("v", literal::i8('B'));
    c.assign("fptr", literal::function_pointer(fnType, "rightEcho"));
    c.callFunctionPointer("fptr", "end").arg("v").done();
  } c.endBlock();

  c.beginBlock("end"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("leftEcho", fnType, {"x"}); {
  c.beginBlock("entry"); {
    c.writeOut(literal::i8('L'));
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("rightEcho", fnType, {"x"}); {
  c.beginBlock("entry"); {
    c.writeOut(literal::i8('R'));
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
