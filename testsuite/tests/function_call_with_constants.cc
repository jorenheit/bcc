// Tests correct argument ordering when constants and i8 locals are interleaved in a function call.
// Expect: AZBY

TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("a", ts::i8());
  c.declareLocal("b", ts::i8());

  c.beginBlock("entry"); {
    c.assign("a", literal::i8('Z'));
    c.assign("b", literal::i8('Y'));

    c.callFunction("foo", "after_foo")(literal::i8('A'), "a", literal::i8('B'), "b");
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = ts::function(ts::voidT())(ts::i8(), ts::i8(), ts::i8(), ts::i8());
c.beginFunction("foo", fooSig, {"p0", "p1", "p2", "p3"}); {
  c.beginBlock("entry"); {
    c.writeOut("p0");
    c.writeOut("p1");
    c.writeOut("p2");
    c.writeOut("p3");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
