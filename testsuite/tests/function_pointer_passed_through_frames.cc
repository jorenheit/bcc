// Tests a function pointer passed through several function frames before it is called.
// main -> pass1 -> pass2 -> pass3, and only pass3 performs the indirect call.
// Expected: Z

TEST_BEGIN

auto voidT = TypeSystem::voidT();
auto actionType = TypeSystem::function(voidT);
auto actionPtr = TypeSystem::function_pointer(actionType);
auto passType = TypeSystem::function(voidT, actionPtr);

c.beginFunction("main"); {
  c.declareLocal("fp", actionPtr);

  c.beginBlock("entry"); {
    c.assign("fp", values::function_pointer(actionType, "printZ"));
    c.callFunction("pass1", "end")("fp");
  } c.endBlock();

  c.beginBlock("end"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("pass1", passType, {"fp"}); {
  c.beginBlock("entry"); {
    c.callFunction("pass2", "done")("fp");
  } c.endBlock();

  c.beginBlock("done"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("pass2", passType, {"fp"}); {
  c.beginBlock("entry"); {
    c.callFunction("pass3", "done")("fp");
  } c.endBlock();

  c.beginBlock("done"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("pass3", passType, {"fp"}); {
  c.beginBlock("entry"); {
    c.callFunctionPointer("fp", "done")();
  } c.endBlock();

  c.beginBlock("done"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("printZ"); {
  c.beginBlock("entry"); {
    c.writeOut(values::i8('Z'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
