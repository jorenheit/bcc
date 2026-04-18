// Tests nested calls with return values, propagating an i8 argument through foo -> bar and back to main.
// Expect: Z

TEST_BEGIN
c.beginFunction("main"); {
  c.declareLocal("r", TypeSystem::i8());

  c.beginBlock("entry"); {
    auto args = c.constructFunctionArguments(values::i8('Z'));
    c.callFunction("foo", "after_foo", args, "r");
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.writeOut("r");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = c.constructFunctionSignature(TypeSystem::i8(),
					   "x", TypeSystem::i8());
c.beginFunction("foo", fooSig); {
  c.declareLocal("tmp", TypeSystem::i8());

  c.beginBlock("entry"); {
    auto args = c.constructFunctionArguments("x");
    c.callFunction("bar", "after_bar", args, "tmp");
  } c.endBlock();

  c.beginBlock("after_bar"); {
    c.returnFromFunction("tmp");
  } c.endBlock();
} c.endFunction();

auto barSig = c.constructFunctionSignature(TypeSystem::i8(), "y", TypeSystem::i8());
c.beginFunction("bar", barSig); {
  c.beginBlock("entry"); {
    c.returnFromFunction("y");
  } c.endBlock();
} c.endFunction();

TEST_END
