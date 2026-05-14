// Tests recursive self-calls with multiple u8 parameters, stressing repeated frame creation and parameter re-passing.
// Expect: ABCABCABCABC...
TEST_BEGIN
c.function("main").begin(); {
  c.callFunction("foo").arg(literal::u8('A')).arg(literal::u8('B')).arg(literal::u8('C')).done();
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("x", ts::u8()).param("y", ts::u8()).param("z", ts::u8()).ret(ts::void_t()).begin(); {
  c.write("x");
  c.write("y");
  c.write("z");

  c.callFunction("foo").arg("x").arg("y").arg("z").done();
  c.returnFromFunction();
} c.endFunction();
TEST_END
