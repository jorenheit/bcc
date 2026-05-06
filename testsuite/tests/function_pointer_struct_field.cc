// Tests a struct containing a function-pointer field.
// The field is assigned, called, overwritten, and called again.
// Expected: AB

TEST_BEGIN

auto voidT = ts::void_t();
auto fnType = ts::function().ret(voidT).done();
auto fnPtr = ts::function_pointer(fnType);

auto holder = ts::defineStruct("Holder").field("fp", fnPtr).done();

c.function("main").begin(); {
  c.declareLocal("h", holder);

  c.assign(c.structField("h", "fp"), literal::function_pointer(fnType, "printA"));
  c.callFunctionPointer(c.structField("h", "fp")).done();
  c.assign(c.structField("h", "fp"), literal::function_pointer(fnType, "printB"));
  c.callFunctionPointer(c.structField("h", "fp")).done();
  c.returnFromFunction();
} c.endFunction();

c.function("printA").begin(); {
  c.writeOut(literal::i8('A'));
  c.returnFromFunction();
} c.endFunction();

c.function("printB").begin(); {
  c.writeOut(literal::i8('B'));
  c.returnFromFunction();
} c.endFunction();

TEST_END
