// Tests basic i8 return-value propagation from callee to caller local.
// Expected: X

TEST_BEGIN

// main: void
c.function("main").begin(); {
  c.declareLocal("x", ts::i8());

  c.callFunction("foo").into("x").done();
  c.write("x");
  c.returnFromFunction();
} c.endFunction();

// foo: returns i8
c.function("foo").ret(ts::i8()).begin(); {

  c.returnFromFunction(literal::i8('X'));
} c.endFunction();

TEST_END
