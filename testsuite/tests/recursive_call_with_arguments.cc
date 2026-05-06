// Tests recursive self-calls with multiple i8 parameters, stressing repeated frame creation and parameter re-passing.
// Expect: ABCABCABCABC...
TEST_BEGIN
c.function("main").begin(); {
  c.callFunction("foo").arg(literal::i8('A')).arg(literal::i8('B')).arg(literal::i8('C')).done();
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("x", ts::i8()).param("y", ts::i8()).param("z", ts::i8()).ret(ts::void_t()).begin(); {
  c.writeOut("x");
  c.writeOut("y");
  c.writeOut("z");

  c.callFunction("foo").arg("x").arg("y").arg("z").done();
  c.returnFromFunction();
} c.endFunction();
TEST_END
