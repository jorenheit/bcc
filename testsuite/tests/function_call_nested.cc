// Tests forwarding an i8 parameter through a nested call chain (main -> foo -> bar).
// Expect: A

TEST_BEGIN
c.function("main").begin(); {
  c.block("entry").begin(); {
    c.callFunction("foo", "after_foo").arg(literal::i8('A')).done();
  } c.endBlock();

  c.block("after_foo").begin(); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("foo").param("x", ts::i8()).ret(ts::void_t()).begin(); {
  c.block("entry").begin(); {
    c.callFunction("bar", "after_bar").arg("x").done();
  } c.endBlock();

  c.block("after_bar").begin(); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("bar").param("y", ts::i8()).ret(ts::void_t()).begin(); {
  c.block("entry").begin(); {
    c.writeOut("y");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
