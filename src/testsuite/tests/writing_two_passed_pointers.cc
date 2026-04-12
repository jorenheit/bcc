// Tests writing through two pointers passed as separate arguments.
// foo reads both pointees, then writes new values through both pointers.
// Expected: ABXY

TEST_BEGIN

auto i8  = TypeSystem::i8();
auto i8p = TypeSystem::pointer(i8);

c.beginFunction("main"); {
  c.declareLocal("pa", i8p);
  c.declareLocal("pb", i8p);
  c.declareLocal("a", i8);
  c.declareLocal("b", i8);

  c.beginBlock("entry"); {
    c.assign("a", values::i8('A'));
    c.assign("b", values::i8('B'));
    c.assign("pa", values::pointer(i8, "a"));
    c.assign("pb", values::pointer(i8, "b"));
    c.callFunction("foo", "after", "pa", "pb");
  } c.endBlock();

  c.beginBlock("after"); {
    c.writeOut("a");
    c.writeOut("b");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("foo", TypeSystem::voidT(), "pa", i8p, "pb", i8p); {
  c.beginBlock("entry"); {
    auto aDeref = c.dereferencePointer("pa");
    auto bDeref = c.dereferencePointer("pb");

    c.writeOut(aDeref);
    c.writeOut(bDeref);

    c.assign(aDeref, values::i8('X'));
    c.assign(bDeref, values::i8('Y'));

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
