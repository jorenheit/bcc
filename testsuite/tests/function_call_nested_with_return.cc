// Tests nested calls with return values, propagating an i8 argument through foo -> bar and back to main.
// Expect: Z

TEST_BEGIN
c.function("main").begin(); {
  c.declareLocal("r", ts::i8());

  c.block("entry").begin(); {
    c.callFunction("foo", "after_foo").into("r").arg(literal::i8('Z')).done();
  } c.endBlock();

  c.block("after_foo").begin(); {
    c.writeOut("r");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("foo").param("x", ts::i8()).ret(ts::i8()).begin(); {
  c.declareLocal("tmp", ts::i8());

  c.block("entry").begin(); {
    c.callFunction("bar", "after_bar").into("tmp").arg("x").done();
  } c.endBlock();

  c.block("after_bar").begin(); {
    c.returnFromFunction("tmp");
  } c.endBlock();
} c.endFunction();

c.function("bar").param("y", ts::i8()).ret(ts::i8()).begin(); {
  c.block("entry").begin(); {
    c.returnFromFunction("y");
  } c.endBlock();
} c.endFunction();

TEST_END
