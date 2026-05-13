// Tests assigning function-pointer literals to a local and then overwriting that local.
// This should catch stale function-pointer target state after reassignment.
// Expected: AB

TEST_BEGIN

auto voidT = ts::void_t();
auto fnType = ts::function().ret(voidT).done();
auto fnPtr = ts::function_pointer(fnType);

c.function("main").begin(); {
  c.declareLocal("fptr", fnPtr);

  c.assign("fptr", literal::function_pointer(fnType, "printA"));
  c.callFunctionPointer("fptr").done();
  c.assign("fptr", literal::function_pointer(fnType, "printB"));
  c.callFunctionPointer("fptr").done();
  c.returnFromFunction();
} c.endFunction();

c.function("printA").begin(); {
  c.write(literal::i8('A'));
  c.returnFromFunction();
} c.endFunction();

c.function("printB").begin(); {
  c.write(literal::i8('B'));
  c.returnFromFunction();
} c.endFunction();

TEST_END
