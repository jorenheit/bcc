// Tests correct argument ordering when constants and i8 locals are interleaved in a function call.
// Expect: AZBY

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("a", ts::i8());
  c.declareLocal("b", ts::i8());

  c.block("entry").begin(); {
    c.assign("a", literal::i8('Z'));
    c.assign("b", literal::i8('Y'));

    c.callFunction("foo", "after_foo").arg(literal::i8('A')).arg("a").arg(literal::i8('B')).arg("b").done();
  } c.endBlock();

  c.block("after_foo").begin(); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("foo").param("p0", ts::i8()).param("p1", ts::i8()).param("p2", ts::i8()).param("p3", ts::i8()).ret(ts::void_t()).begin(); {
  c.block("entry").begin(); {
    c.writeOut("p0");
    c.writeOut("p1");
    c.writeOut("p2");
    c.writeOut("p3");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
