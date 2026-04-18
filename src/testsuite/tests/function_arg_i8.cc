// Tests passing an i8 constant and an i8 local as function arguments, and returning an i8 value into a caller local.
// Expected: AB

TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("x", TypeSystem::i8());
  c.declareLocal("y", TypeSystem::i8());
       
  c.beginBlock("entry"); {
    c.assign("x", values::i8('A'));
    auto args = c.constructFunctionArguments("x", values::i8('B'));
    c.callFunction("foo", "after_foo", args, "y");
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.writeOut("x");
    c.writeOut("y");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = c.constructFunctionSignature(TypeSystem::i8(),
					   "arg1", TypeSystem::i8(),
					   "arg2", TypeSystem::i8());

c.beginFunction("foo", fooSig); {
  c.beginBlock("entry"); {
    c.returnFromFunction("arg2");
  } c.endBlock();
} c.endFunction();

TEST_END
