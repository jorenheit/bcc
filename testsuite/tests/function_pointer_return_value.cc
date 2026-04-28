// Tests calling function pointers with an i8 argument and an i8 return value.
// The return value is stored in a local supplied to callFunctionPointer and then written out.
// Expected: BA

TEST_BEGIN

auto i8 = ts::i8();
auto fnType = ts::function(i8)(i8);
auto fnPtr = ts::function_pointer(fnType);

c.beginFunction("main"); {
  c.declareLocal("fptr", fnPtr);
  c.declareLocal("r", i8);

  c.beginBlock("entry"); {
    c.assign("fptr", literal::function_pointer(fnType, "inc"));
    c.callFunctionPointer("fptr", "second", "r")(literal::i8('A'));
  } c.endBlock();

  c.beginBlock("second"); {
    c.writeOut("r");
    c.assign("fptr", literal::function_pointer(fnType, "dec"));
    c.callFunctionPointer("fptr", "end", "r")(literal::i8('B'));
  } c.endBlock();

  c.beginBlock("end"); {
    c.writeOut("r");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("inc", fnType, {"x"}); {
  c.beginBlock("entry"); {
    c.returnFromFunction(c.add("x", literal::i8(1)));
  } c.endBlock();
} c.endFunction();

c.beginFunction("dec", fnType, {"x"}); {
  c.beginBlock("entry"); {
    c.returnFromFunction(c.sub("x", literal::i8(1)));
  } c.endBlock();
} c.endFunction();

TEST_END
