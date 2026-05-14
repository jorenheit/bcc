// Tests calling function pointers with u8 arguments.
// The two targets both receive an argument, but add their own prefix so dispatch and argument transport are both checked.
// Expected: LARB

TEST_BEGIN

auto u8 = ts::u8();
auto voidT = ts::void_t();
auto fnType = ts::function().ret(voidT).param(u8).done();
auto fnPtr = ts::function_pointer(fnType);

c.function("main").begin(); {
  c.declareLocal("fptr", fnPtr);
  c.declareLocal("v", u8);

  c.assign("v", literal::u8('A'));
  c.assign("fptr", literal::function_pointer(fnType, "leftEcho"));
  c.callFunctionPointer("fptr").arg("v").done();
  c.assign("v", literal::u8('B'));
  c.assign("fptr", literal::function_pointer(fnType, "rightEcho"));
  c.callFunctionPointer("fptr").arg("v").done();
  c.returnFromFunction();
} c.endFunction();

c.function("leftEcho").param("x", u8).ret(voidT).begin(); {
  c.write(literal::u8('L'));
  c.write("x");
  c.returnFromFunction();
} c.endFunction();

c.function("rightEcho").param("x", u8).ret(voidT).begin(); {
  c.write(literal::u8('R'));
  c.write("x");
  c.returnFromFunction();
} c.endFunction();

TEST_END
