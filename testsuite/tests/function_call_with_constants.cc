// Tests correct argument ordering when constants and u8 locals are interleaved in a function call.
// Expect: AZBY

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("a", ts::u8());
  c.declareLocal("b", ts::u8());

  c.assign("a", literal::u8('Z'));
  c.assign("b", literal::u8('Y'));

  c.callFunction("foo").arg(literal::u8('A')).arg("a").arg(literal::u8('B')).arg("b").done();
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("p0", ts::u8()).param("p1", ts::u8()).param("p2", ts::u8()).param("p3", ts::u8()).ret(ts::void_t()).begin(); {
  c.write("p0");
  c.write("p1");
  c.write("p2");
  c.write("p3");
  c.returnFromFunction();
} c.endFunction();

TEST_END
