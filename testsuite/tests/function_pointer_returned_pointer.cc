// Tests returning function pointers from another function and calling both returned targets in one program.
// This stresses function-pointer return transport and subsequent indirect calls.
// Expected: TF

TEST_BEGIN

auto i8 = ts::i8();
auto voidT = ts::void_t();
auto targetType = ts::function().ret(voidT).done();
auto targetPtr = ts::function_pointer(targetType);

auto chooserType = ts::function().ret(targetPtr).param(i8).done();

c.beginFunction("main"); {
  c.declareLocal("fptr", targetPtr);

  c.beginBlock("entry"); {
    c.callFunction("choose", "call_true").into("fptr").arg(literal::i8(1)).done();
  } c.endBlock();

  c.beginBlock("call_true"); {
    c.callFunctionPointer("fptr", "choose_false").done();
  } c.endBlock();

  c.beginBlock("choose_false"); {
    c.callFunction("choose", "call_false").into("fptr").arg(literal::i8(0)).done();
  } c.endBlock();

  c.beginBlock("call_false"); {
    c.callFunctionPointer("fptr", "end").done();
  } c.endBlock();

  c.beginBlock("end"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("choose", chooserType, {"flag"}); {
  c.beginBlock("entry"); {
    c.branchIf("flag", "true", "false");
  } c.endBlock();

  c.beginBlock("true"); {
    c.returnFromFunction(literal::function_pointer(targetType, "printT"));
  } c.endBlock();

  c.beginBlock("false"); {
    c.returnFromFunction(literal::function_pointer(targetType, "printF"));
  } c.endBlock();
} c.endFunction();

c.beginFunction("printT"); {
  c.beginBlock("entry"); {
    c.writeOut(literal::i8('T'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("printF"); {
  c.beginBlock("entry"); {
    c.writeOut(literal::i8('F'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
