// Tests writing through two pointers passed as separate arguments.
// foo reads both pointees, then writes new values through both pointers.
// Expected: ABXY

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);

c.beginFunction("main"); {
  c.declareLocal("pa", i8p);
  c.declareLocal("pb", i8p);
  c.declareLocal("a", i8);
  c.declareLocal("b", i8);

  c.beginBlock("entry"); {
    c.assign("a", literal::i8('A'));
    c.assign("b", literal::i8('B'));
    c.assign("pa", c.addressOf("a"));
    c.assign("pb", c.addressOf("b"));

    c.callFunction("foo", "after")("pa", "pb");
  } c.endBlock();

  c.beginBlock("after"); {
    c.writeOut("a");
    c.writeOut("b");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto sig = ts::function(ts::voidT())(i8p, i8p);
c.beginFunction("foo", sig, {"pa", "pb"}); {
  c.beginBlock("entry"); {
    auto aDeref = c.dereferencePointer("pa");
    auto bDeref = c.dereferencePointer("pb");

    c.writeOut(aDeref);
    c.writeOut(bDeref);

    c.assign(aDeref, literal::i8('X'));
    c.assign(bDeref, literal::i8('Y'));

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
