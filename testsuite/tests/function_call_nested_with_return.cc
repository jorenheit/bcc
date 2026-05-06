// Tests nested calls with return values, propagating an i8 argument through foo -> bar and back to main.
// Expect: Z

TEST_BEGIN
c.function("main").begin(); {
  c.declareLocal("r", ts::i8());

  c.callFunction("foo").into("r").arg(literal::i8('Z')).done();
  c.writeOut("r");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("x", ts::i8()).ret(ts::i8()).begin(); {
  c.declareLocal("tmp", ts::i8());

  c.callFunction("bar").into("tmp").arg("x").done();
  c.returnFromFunction("tmp");
} c.endFunction();

c.function("bar").param("y", ts::i8()).ret(ts::i8()).begin(); {
  c.returnFromFunction("y");
} c.endFunction();

TEST_END
