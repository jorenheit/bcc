// Tests calling function pointers with an i8 argument and an i8 return value.
// The return value is stored in a local supplied to callFunctionPointer and then written out.
// Expected: BA

TEST_BEGIN

auto i8 = ts::i8();
auto fnType = ts::function().ret(i8).param(i8).done();
auto fnPtr = ts::function_pointer(fnType);

c.function("main").begin(); {
  c.declareLocal("fptr", fnPtr);
  c.declareLocal("r", i8);

  c.assign("fptr", literal::function_pointer(fnType, "inc"));
  c.callFunctionPointer("fptr").into("r").arg(literal::i8('A')).done();
  c.writeOut("r");
  c.assign("fptr", literal::function_pointer(fnType, "dec"));
  c.callFunctionPointer("fptr").into("r").arg(literal::i8('B')).done();
  c.writeOut("r");
  c.returnFromFunction();
} c.endFunction();

c.function("inc").param("x", i8).ret(i8).begin(); {
  c.returnFromFunction(c.add("x", literal::i8(1)));
} c.endFunction();

c.function("dec").param("x", i8).ret(i8).begin(); {
  c.returnFromFunction(c.sub("x", literal::i8(1)));
} c.endFunction();

TEST_END
