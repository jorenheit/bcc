// Tests passing i16 arguments (constant and local) and returning an i16 value from the callee.
// Expect: AB
TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("x", ts::i16());
  c.declareLocal("y", ts::i16());

  c.beginBlock("entry"); {
    c.assign("x", literal::i16(CAT('C', 'D')));
    c.callFunction("foo", "after_foo", "y")("x", literal::i16(CAT('A', 'B')));
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.writeOut("y");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = ts::function(ts::i16())(ts::i16(), ts::i16());
c.beginFunction("foo", fooSig, {"arg1", "arg2"}); {
  c.beginBlock("entry"); {
    c.returnFromFunction("arg2");
  } c.endBlock();
} c.endFunction();

TEST_END
