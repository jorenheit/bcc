// Tests nested calls with return values, propagating an i8 argument through foo -> bar and back to main.
// Expect: Z

TEST_BEGIN
c.beginFunction("main"); {
  c.declareLocal("r", TypeSystem::i8());

  c.beginBlock("entry"); {
    c.callFunctionReturn("foo", "after_foo", "r",  values::i8('Z'));
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.writeOut("r");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("foo", TypeSystem::i8(), "x", TypeSystem::i8()); {
  c.declareLocal("tmp", TypeSystem::i8());

  c.beginBlock("entry"); {
    c.callFunctionReturn("bar", "after_bar", "tmp", "x");
  } c.endBlock();

  c.beginBlock("after_bar"); {
    c.returnFromFunction("tmp");
  } c.endBlock();
} c.endFunction();

c.beginFunction("bar", TypeSystem::i8(), "y", TypeSystem::i8()); {
  c.beginBlock("entry"); {
    c.returnFromFunction("y");
  } c.endBlock();
} c.endFunction();

TEST_END
