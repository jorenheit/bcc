// Tests basic i8 return-value propagation from callee to caller local.
// Expected: X

TEST_BEGIN

// main: void
c.beginFunction("main"); {
  c.declareLocal("x", TypeSystem::i8());

  c.beginBlock("entry"); {
    c.callFunctionReturn("foo", "after_foo", "x");
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();

} c.endFunction();

// foo: returns i8
c.beginFunction("foo", TypeSystem::i8()); {

  c.beginBlock("entry"); {
    c.returnFromFunction(values::i8('X'));
  } c.endBlock();

} c.endFunction();

TEST_END
