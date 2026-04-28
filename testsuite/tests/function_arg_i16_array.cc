// Tests pass-by-value of an array of i16 values, verifying multi-cell argument copying across a function call.
// Expect: ABCDABCD

TEST_BEGIN
auto array2 = ts::array(ts::i16(), 2);

c.beginFunction("main"); {
  c.declareLocal("x", array2);

  c.beginBlock("entry"); {
    auto x0 = c.arrayElement("x", 0);
    auto x1 = c.arrayElement("x", 1);

    c.assign(x0, literal::i16(CAT('A', 'B')));
    c.assign(x1, literal::i16(CAT('C', 'D')));

    c.writeOut("x");
    c.callFunction("foo", "after_foo")("x");
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = ts::function(ts::voidT())(array2);
c.beginFunction("foo", fooSig, {"x"}); {
  c.beginBlock("entry"); {
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
