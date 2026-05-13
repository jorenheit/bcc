// Tests forwarding an i8 parameter through a nested call chain (main -> foo -> bar).
// Expect: A

TEST_BEGIN
c.function("main").begin(); {
  c.callFunction("foo").arg(literal::i8('A')).done();
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("x", ts::i8()).ret(ts::void_t()).begin(); {
  c.callFunction("bar").arg("x").done();
  c.returnFromFunction();
} c.endFunction();

c.function("bar").param("y", ts::i8()).ret(ts::void_t()).begin(); {
  c.write("y");
  c.returnFromFunction();
} c.endFunction();

TEST_END
