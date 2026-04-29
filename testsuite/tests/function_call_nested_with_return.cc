// Tests nested calls with return values, propagating an i8 argument through foo -> bar and back to main.
// Expect: Z

TEST_BEGIN
c.beginFunction("main"); {
  c.declareLocal("r", ts::i8());

  c.beginBlock("entry"); {
    c.callFunction("foo", "after_foo").into("r").arg(literal::i8('Z')).done();
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.writeOut("r");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = ts::function().ret(ts::i8()).param(ts::i8()).done();
c.beginFunction("foo", fooSig, {"x"}); {
  c.declareLocal("tmp", ts::i8());

  c.beginBlock("entry"); {
    c.callFunction("bar", "after_bar").into("tmp").arg("x").done();
  } c.endBlock();

  c.beginBlock("after_bar"); {
    c.returnFromFunction("tmp");
  } c.endBlock();
} c.endFunction();

auto barSig = ts::function().ret(ts::i8()).param(ts::i8()).done();
c.beginFunction("bar", barSig, {"y"}); {
  c.beginBlock("entry"); {
    c.returnFromFunction("y");
  } c.endBlock();
} c.endFunction();

TEST_END
