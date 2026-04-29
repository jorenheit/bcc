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

  c.block("entry").begin(); {
    c.callFunction("getPtr", "call_false").into("fptr").arg(literal::i8(0)).done();
  } c.endBlock();

  c.block("call_false").begin(); {
    c.callFunctionPointer("fptr", "get_true").done();
  } c.endBlock();

  c.block("get_true").begin(); {
    c.callFunction("getPtr", "call_true").into("fptr").arg(literal::i8(1)).done();
  } c.endBlock();

  c.block("call_true").begin(); {
    c.callFunctionPointer("fptr", "end").done();
  } c.endBlock();

  c.block("end").begin(); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();


c.function("getPtr").param("x", i8).ret(fooPtr).begin(); {
  c.block("entry").begin(); {
    c.branchIf("x", "true", "false");
  } c.endBlock();

  c.block("true").begin(); {
    c.returnFromFunction(literal::function_pointer(fooType, "foo1"));
  } c.endBlock();

  c.block("false").begin(); {
    c.returnFromFunction(literal::function_pointer(fooType, "foo2"));
  } c.endBlock();
} c.endFunction();

c.function("foo1").begin(); {
  c.block("entry").begin(); {
    c.writeOut(literal::string("foo1"));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("foo2").begin(); {
  c.block("entry").begin(); {
    c.writeOut(literal::string("foo2"));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
