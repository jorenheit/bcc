// Tests writing through two pointers passed as separate arguments.
// foo reads both pointees, then writes new values through both pointers.
// Expected: ABXY

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);

c.function("main").begin(); {
  c.declareLocal("pa", i8p);
  c.declareLocal("pb", i8p);
  c.declareLocal("a", i8);
  c.declareLocal("b", i8);

  c.assign("a", literal::i8('A'));
  c.assign("b", literal::i8('B'));
  c.assign("pa", c.addressOf("a"));
  c.assign("pb", c.addressOf("b"));

  c.callFunction("foo").arg("pa").arg("pb").done();
  c.writeOut("a");
  c.writeOut("b");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("pa", i8p).param("pb", i8p).ret(ts::void_t()).begin(); {
  auto aDeref = c.dereferencePointer("pa");
  auto bDeref = c.dereferencePointer("pb");

  c.writeOut(aDeref);
  c.writeOut(bDeref);

  c.assign(aDeref, literal::i8('X'));
  c.assign(bDeref, literal::i8('Y'));

  c.returnFromFunction();
} c.endFunction();

TEST_END
