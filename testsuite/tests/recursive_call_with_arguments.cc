// Tests recursive self-calls with multiple i8 parameters, stressing repeated frame creation and parameter re-passing.
// Expect: ABCABCABCABC...
TEST_BEGIN
c.function("main").begin(); {
  c.block("entry").begin(); {
    
    c.callFunction("foo", "after_foo").arg(literal::i8('A')).arg(literal::i8('B')).arg(literal::i8('C')).done();
  } c.endBlock();

  c.block("after_foo").begin(); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("foo").param("x", ts::i8()).param("y", ts::i8()).param("z", ts::i8()).ret(ts::void_t()).begin(); {
  c.block("entry").begin(); {
    c.writeOut("x");
    c.writeOut("y");
    c.writeOut("z");

    c.callFunction("foo", "after_recurse").arg("x").arg("y").arg("z").done();
  } c.endBlock();

  c.block("after_recurse").begin(); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();
TEST_END
