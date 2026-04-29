// Tests returning function pointers from another function and calling both returned targets in one program.
// This stresses function-pointer return transport and subsequent indirect calls.
// Expected: TF

TEST_BEGIN

auto i8 = ts::i8();
auto voidT = ts::void_t();
auto targetType = ts::function().ret(voidT).done();
auto targetPtr = ts::function_pointer(targetType);

c.function("main").begin(); {
  c.declareLocal("fptr", targetPtr);

  c.block("entry").begin(); {
    c.callFunction("choose", "call_true").into("fptr").arg(literal::i8(1)).done();
  } c.endBlock();

  c.block("call_true").begin(); {
    c.callFunctionPointer("fptr", "choose_false").done();
  } c.endBlock();

  c.block("choose_false").begin(); {
    c.callFunction("choose", "call_false").into("fptr").arg(literal::i8(0)).done();
  } c.endBlock();

  c.block("call_false").begin(); {
    c.callFunctionPointer("fptr", "end").done();
  } c.endBlock();

  c.block("end").begin(); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("choose").param("flag", i8).ret(targetPtr).begin(); {
  c.block("entry").begin(); {
    c.branchIf("flag", "true", "false");
  } c.endBlock();

  c.block("true").begin(); {
    c.returnFromFunction(literal::function_pointer(targetType, "printT"));
  } c.endBlock();

  c.block("false").begin(); {
    c.returnFromFunction(literal::function_pointer(targetType, "printF"));
  } c.endBlock();
} c.endFunction();

c.function("printT").begin(); {
  c.block("entry").begin(); {
    c.writeOut(literal::i8('T'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("printF").begin(); {
  c.block("entry").begin(); {
    c.writeOut(literal::i8('F'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
