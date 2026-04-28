// Tests repeated pointer-depth adjustment across two call boundaries.
// main creates a pointer to x, foo forwards it to bar, and bar reads/writes through it.
// Expected: AX

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);

c.beginFunction("main"); {
  c.declareLocal("p", i8p);
  c.declareLocal("x", i8);

  c.beginBlock("entry"); {
    c.assign("x", literal::i8('A'));
    c.assign("p", c.addressOf("x"));
    c.callFunction("foo", "after")("p");
  } c.endBlock();

  c.beginBlock("after"); {
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto sig = ts::function(ts::voidT())(i8p);
c.beginFunction("foo", sig, {"p"}); {
  c.beginBlock("entry"); {
    c.callFunction("bar", "return")("p");
  } c.endBlock();

  c.beginBlock("return"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("bar", sig, {"p"}); {
  c.beginBlock("entry"); {
    auto pDeref = c.dereferencePointer("p");
    c.writeOut(pDeref);
    c.assign(pDeref, literal::i8('X'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
