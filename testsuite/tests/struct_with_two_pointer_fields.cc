// Tests a struct containing two pointer fields passed to a function.
// foo reads through both pointers, then writes through both.
// Expected: ABXY

TEST_BEGIN

auto u8  = ts::u8();
auto u8p = ts::pointer(u8);

auto pair = ts::defineStruct("Pair").field("pa", u8p).field("pb", u8p).done();

c.function("main").begin(); {
  c.declareLocal("s", pair);
  c.declareLocal("a", u8);
  c.declareLocal("b", u8);

  c.assign("a", literal::u8('A'));
  c.assign("b", literal::u8('B'));

  c.assign(c.structField("s", "pa"), c.addressOf("a"));
  c.assign(c.structField("s", "pb"), c.addressOf("b"));

  c.callFunction("foo").arg("s").done();
  c.write("a");
  c.write("b");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("s", pair).ret(ts::void_t()).begin(); {
  auto pa = c.structField("s", "pa");
  auto pb = c.structField("s", "pb");

  auto aDeref = c.dereferencePointer(pa);
  auto bDeref = c.dereferencePointer(pb);

  c.write(aDeref);
  c.write(bDeref);

  c.assign(aDeref, literal::u8('X'));
  c.assign(bDeref, literal::u8('Y'));

  c.returnFromFunction();
} c.endFunction();

TEST_END
