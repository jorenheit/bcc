// Tests writing through two pointers passed as separate arguments.
// foo reads both pointees, then writes new values through both pointers.
// Expected: ABXY

TEST_BEGIN

auto u8  = ts::u8();
auto u8p = ts::pointer(u8);

c.function("main").begin(); {
  c.declareLocal("pa", u8p);
  c.declareLocal("pb", u8p);
  c.declareLocal("a", u8);
  c.declareLocal("b", u8);

  c.assign("a", literal::u8('A'));
  c.assign("b", literal::u8('B'));
  c.assign("pa", c.addressOf("a"));
  c.assign("pb", c.addressOf("b"));

  c.callFunction("foo").arg("pa").arg("pb").done();
  c.write("a");
  c.write("b");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("pa", u8p).param("pb", u8p).ret(ts::void_t()).begin(); {
  auto aDeref = c.dereferencePointer("pa");
  auto bDeref = c.dereferencePointer("pb");

  c.write(aDeref);
  c.write(bDeref);

  c.assign(aDeref, literal::u8('X'));
  c.assign(bDeref, literal::u8('Y'));

  c.returnFromFunction();
} c.endFunction();

TEST_END
