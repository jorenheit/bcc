// Tests direct function-pointer selection through a function returning a function pointer.
// getPtr(0) returns foo2, getPtr(1) returns foo1, and both pointers are called.
// Expected: foo2foo1

TEST_BEGIN

auto i8 = ts::i8();
auto voidT = ts::void_t();
auto fooType = ts::function().ret(voidT).done();
auto fooPtr = ts::function_pointer(fooType);

c.function("main").begin(); {
  c.declareLocal("fptr", fooPtr);

  c.callFunction("getPtr").into("fptr").arg(literal::i8(0)).done();
  c.callFunctionPointer("fptr").done();
  c.callFunction("getPtr").into("fptr").arg(literal::i8(1)).done();
  c.callFunctionPointer("fptr").done();
  c.returnFromFunction();
} c.endFunction();


c.function("getPtr").param("x", i8).ret(fooPtr).begin(); {
  c.jumpIf("x", "true", "false");
  c.label("true");
  c.returnFromFunction(literal::function_pointer(fooType, "foo1"));
  c.label("false");
  c.returnFromFunction(literal::function_pointer(fooType, "foo2"));
} c.endFunction();

c.function("foo1").begin(); {
  c.writeOut(literal::string("foo1"));
  c.returnFromFunction();
} c.endFunction();

c.function("foo2").begin(); {
  c.writeOut(literal::string("foo2"));
  c.returnFromFunction();
} c.endFunction();

TEST_END
