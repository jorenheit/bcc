// Tests writing through a pointer that points to a caller local and is passed to a function.
// foo writes 'Y' through p, so x changes from 'X' to 'Y'.
// Expected: YY

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);

c.beginFunction("main"); {
  c.declareLocal("p", i8p);
  c.declareLocal("x", i8);

  c.beginBlock("entry"); {
    c.assign("x", literal::i8('X'));
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
    auto pDeref = c.dereferencePointer("p");
    c.assign(pDeref, literal::i8('Y'));
    c.writeOut(pDeref);
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
