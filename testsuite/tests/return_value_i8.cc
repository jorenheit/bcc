// Tests basic i8 return-value propagation from callee to caller local.
// Expected: X

TEST_BEGIN

// main: void
c.function("main").begin(); {
  c.declareLocal("x", ts::i8());

  c.block("entry").begin(); {
    c.callFunction("foo", "after_foo").into("x").done();
  } c.endBlock();

  c.block("after_foo").begin(); {
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();

} c.endFunction();

// foo: returns i8
c.function("foo").ret(ts::i8()).begin(); {

  c.block("entry").begin(); {
    c.returnFromFunction(literal::i8('X'));
  } c.endBlock();

} c.endFunction();

TEST_END
