// Tests a struct containing two pointer fields passed to a function.
// foo reads through both pointers, then writes through both.
// Expected: ABXY

TEST_BEGIN

auto i8  = TypeSystem::i8();
auto i8p = TypeSystem::pointer(i8);

auto pair = c.defineStruct("Pair",
                           "pa", i8p,
                           "pb", i8p);

c.beginFunction("main"); {
  c.declareLocal("s", pair);
  c.declareLocal("a", i8);
  c.declareLocal("b", i8);

  c.beginBlock("entry"); {
    c.assign("a", values::i8('A'));
    c.assign("b", values::i8('B'));

    c.assign(c.structField("s", "pa"), values::pointer(i8, "a"));
    c.assign(c.structField("s", "pb"), values::pointer(i8, "b"));

    c.callFunction("foo", "after", "s");
  } c.endBlock();

  c.beginBlock("after"); {
    c.writeOut("a");
    c.writeOut("b");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("foo", TypeSystem::voidT(), "s", pair); {
  c.beginBlock("entry"); {
    auto pa = c.structField("s", "pa");
    auto pb = c.structField("s", "pb");

    auto aDeref = c.dereferencePointer(pa);
    auto bDeref = c.dereferencePointer(pb);

    c.writeOut(aDeref);
    c.writeOut(bDeref);

    c.assign(aDeref, values::i8('X'));
    c.assign(bDeref, values::i8('Y'));

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
