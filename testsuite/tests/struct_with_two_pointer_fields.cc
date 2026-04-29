// Tests a struct containing two pointer fields passed to a function.
// foo reads through both pointers, then writes through both.
// Expected: ABXY

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);

auto pair = ts::defineStruct("Pair").field("pa", i8p).field("pb", i8p).done();

c.function("main").begin(); {
  c.declareLocal("s", pair);
  c.declareLocal("a", i8);
  c.declareLocal("b", i8);

  c.block("entry").begin(); {
    c.assign("a", literal::i8('A'));
    c.assign("b", literal::i8('B'));

    c.assign(c.structField("s", "pa"), c.addressOf("a"));
    c.assign(c.structField("s", "pb"), c.addressOf("b"));

    c.callFunction("foo", "after").arg("s").done();
  } c.endBlock();

  c.block("after").begin(); {
    c.writeOut("a");
    c.writeOut("b");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("foo").param("s", pair).ret(ts::void_t()).begin(); {
  c.block("entry").begin(); {
    auto pa = c.structField("s", "pa");
    auto pb = c.structField("s", "pb");

    auto aDeref = c.dereferencePointer(pa);
    auto bDeref = c.dereferencePointer(pb);

    c.writeOut(aDeref);
    c.writeOut(bDeref);

    c.assign(aDeref, literal::i8('X'));
    c.assign(bDeref, literal::i8('Y'));

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
