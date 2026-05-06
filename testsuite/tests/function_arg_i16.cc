// Tests passing i16 arguments (constant and local) and returning an i16 value from the callee.
// Expect: AB
TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i16());
  c.declareLocal("y", ts::i16());

  c.assign("x", literal::i16(CAT('C', 'D')));
  c.callFunction("foo").into("y").arg("x").arg(literal::i16(CAT('A', 'B'))).done();
  c.writeOut("y");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("arg1", ts::i16()).param("arg2", ts::i16()).ret(ts::i16()).begin(); {
  c.returnFromFunction("arg2");
} c.endFunction();

TEST_END
