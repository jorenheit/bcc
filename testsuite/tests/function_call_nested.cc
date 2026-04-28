// Tests forwarding an i8 parameter through a nested call chain (main -> foo -> bar).
// Expect: A

TEST_BEGIN
c.beginFunction("main"); {
  c.beginBlock("entry"); {
    c.callFunction("foo", "after_foo")(literal::i8('A'));
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = ts::function(ts::voidT())(ts::i8());
c.beginFunction("foo", fooSig, {"x"}); {
  c.beginBlock("entry"); {
    c.callFunction("bar", "after_bar")("x");
  } c.endBlock();

  c.beginBlock("after_bar"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto barSig = ts::function(ts::voidT())(ts::i8());
c.beginFunction("bar", barSig, {"y"}); {
  c.beginBlock("entry"); {
    c.writeOut("y");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
