// Tests recursive self-calls with multiple i8 parameters, stressing repeated frame creation and parameter re-passing.
// Expect: ABCABCABCABC...
TEST_BEGIN
c.beginFunction("main"); {
  c.beginBlock("entry"); {
    c.callFunction("foo", "after_foo",
		   values::i8('A'),
		   values::i8('B'),
		   values::i8('C'));
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("foo", TypeSystem::voidT(),
		"x", TypeSystem::i8(),
		"y", TypeSystem::i8(),
		"z", TypeSystem::i8()); {
  c.beginBlock("entry"); {
    c.writeOut("x");
    c.writeOut("y");
    c.writeOut("z");

    c.callFunction("foo", "after_recurse", "x",
		   "y", "z");
  } c.endBlock();

  c.beginBlock("after_recurse"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();
TEST_END
