// Tests a struct containing a function-pointer field.
// The field is assigned, called, overwritten, and called again.
// Expected: AB

TEST_BEGIN

auto voidT = TypeSystem::voidT();
auto fnType = TypeSystem::function(voidT);
auto fnPtr = TypeSystem::function_pointer(fnType);

auto holder = c.defineStruct("Holder")("fp", fnPtr);

c.beginFunction("main"); {
  c.declareLocal("h", holder);

  c.beginBlock("entry"); {
    c.assign(c.structField("h", "fp"), values::function_pointer(fnType, "printA"));
    c.callFunctionPointer(c.structField("h", "fp"), "second")();
  } c.endBlock();

  c.beginBlock("second"); {
    c.assign(c.structField("h", "fp"), values::function_pointer(fnType, "printB"));
    c.callFunctionPointer(c.structField("h", "fp"), "end")();
  } c.endBlock();

  c.beginBlock("end"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("printA"); {
  c.beginBlock("entry"); {
    c.writeOut(values::i8('A'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("printB"); {
  c.beginBlock("entry"); {
    c.writeOut(values::i8('B'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
