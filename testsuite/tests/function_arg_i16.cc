// Tests passing i16 arguments (constant and local) and returning an i16 value from the callee.
// Expect: AB
TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i16());
  c.declareLocal("y", ts::i16());

  c.block("entry").begin(); {
    c.assign("x", literal::i16(CAT('C', 'D')));
    c.callFunction("foo", "after_foo").into("y").arg("x").arg(literal::i16(CAT('A', 'B'))).done();
  } c.endBlock();

  c.block("after_foo").begin(); {
    c.writeOut("y");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("foo").param("arg1", ts::i16()).param("arg2", ts::i16()).ret(ts::i16()).begin(); {
  c.block("entry").begin(); {
    c.returnFromFunction("arg2");
  } c.endBlock();
} c.endFunction();

TEST_END
