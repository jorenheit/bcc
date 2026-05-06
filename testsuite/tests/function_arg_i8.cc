// Tests passing an i8 constant and an i8 local as function arguments, and returning an i8 value into a caller local.
// Expected: AB

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i8());
  c.declareLocal("y", ts::i8());

  c.assign("x", literal::i8('A'));
  c.callFunction("foo").into("y").arg("x").arg(literal::i8('B')).done();
  c.writeOut("x");
  c.writeOut("y");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("arg1", ts::i8()).param("arg2", ts::i8()).ret(ts::i8()).begin(); {
  c.returnFromFunction("arg2");
} c.endFunction();

TEST_END
