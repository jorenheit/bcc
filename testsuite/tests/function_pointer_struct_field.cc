// Tests a struct containing a function-pointer field.
// The field is assigned, called, overwritten, and called again.
// Expected: AB

TEST_BEGIN

auto voidT = ts::void_t();
auto fnType = ts::function().ret(voidT).done();
auto fnPtr = ts::function_pointer(fnType);

auto holder = ts::defineStruct("Holder").field("fp", fnPtr).done();

c.beginFunction("main"); {
  c.declareLocal("h", holder);

  c.beginBlock("entry"); {
    c.assign(c.structField("h", "fp"), literal::function_pointer(fnType, "printA"));
    c.callFunctionPointer(c.structField("h", "fp"), "second").done();
  } c.endBlock();

  c.beginBlock("second"); {
    c.assign(c.structField("h", "fp"), literal::function_pointer(fnType, "printB"));
    c.callFunctionPointer(c.structField("h", "fp"), "end").done();
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
