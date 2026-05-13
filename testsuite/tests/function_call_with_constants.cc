// Tests correct argument ordering when constants and i8 locals are interleaved in a function call.
// Expect: AZBY

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("a", ts::i8());
  c.declareLocal("b", ts::i8());

  c.assign("a", literal::i8('Z'));
  c.assign("b", literal::i8('Y'));

  c.callFunction("foo").arg(literal::i8('A')).arg("a").arg(literal::i8('B')).arg("b").done();
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("p0", ts::i8()).param("p1", ts::i8()).param("p2", ts::i8()).param("p3", ts::i8()).ret(ts::void_t()).begin(); {
  c.write("p0");
  c.write("p1");
  c.write("p2");
  c.write("p3");
  c.returnFromFunction();
} c.endFunction();

TEST_END
