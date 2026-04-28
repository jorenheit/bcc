// Passing entire struct object to function
// Expected: ABABXYAB

TEST_BEGIN

auto point = ts::defineStruct("Point")("x", ts::i8(),
				     "y", ts::i8());

c.beginFunction("main"); {
  c.declareLocal("s", point);

  c.beginBlock("entry"); {
    auto x = c.structField("s", "x");
    auto y = c.structField("s", "y");

    c.assign(x, literal::i8('A'));
    c.assign(y, literal::i8('B'));
    c.writeOut("s");

    c.callFunction("foo", "after_foo")("s");
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = ts::function(ts::voidT())(point);
c.beginFunction("foo", fooSig, {"p"}); {
  c.beginBlock("entry"); {
    auto px = c.structField("p", "x");
    auto py = c.structField("p", "y");

    c.writeOut("p");
    c.assign(px, literal::i8('X'));
    c.assign(py, literal::i8('Y'));
    c.writeOut("p");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
