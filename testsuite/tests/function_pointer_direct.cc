// Tests direct function-pointer selection through a function returning a function pointer.
// getPtr(0) returns foo2, getPtr(1) returns foo1, and both pointers are called.
// Expected: foo2foo1

TEST_BEGIN

auto i8 = ts::i8();
auto voidT = ts::void_t();
auto fooType = ts::function().ret(voidT).done();
auto fooPtr = ts::function_pointer(fooType);

c.beginFunction("main"); {
  c.declareLocal("fptr", fooPtr);

  c.beginBlock("entry"); {
    c.callFunction("getPtr", "call_false").into("fptr").arg(literal::i8(0)).done();
  } c.endBlock();

  c.beginBlock("call_false"); {
    c.callFunctionPointer("fptr", "get_true").done();
  } c.endBlock();

  c.beginBlock("get_true"); {
    c.callFunction("getPtr", "call_true").into("fptr").arg(literal::i8(1)).done();
  } c.endBlock();

  c.beginBlock("call_true"); {
    c.callFunctionPointer("fptr", "end").done();
  } c.endBlock();

  c.beginBlock("end"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();


auto getPtrType = ts::function().ret(fooPtr).param(i8).done();
c.beginFunction("getPtr", getPtrType, {"x"}); {
  c.beginBlock("entry"); {
    c.branchIf("x", "true", "false");
  } c.endBlock();

  c.beginBlock("true"); {
    c.returnFromFunction(literal::function_pointer(fooType, "foo1"));
  } c.endBlock();

  c.beginBlock("false"); {
    c.returnFromFunction(literal::function_pointer(fooType, "foo2"));
  } c.endBlock();
} c.endFunction();

c.beginFunction("foo1"); {
  c.beginBlock("entry"); {
    c.writeOut(literal::string("foo1"));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("foo2"); {
  c.beginBlock("entry"); {
    c.writeOut(literal::string("foo2"));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
