// Tests a struct containing a pointer field passed to a function.
// foo reads through s.p, then writes through it. main observes the updated pointee.
// Expected: AAX

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);

auto holder = ts::defineStruct("Holder").field("p", i8p).done();

c.function("main").begin(); {
  c.declareLocal("s", holder);
  c.declareLocal("x", i8);

  c.assign("x", literal::i8('A'));
  c.assign(c.structField("s", "p"), c.addressOf("x"));

  c.callFunction("foo").arg("s").done();
  c.writeOut("x");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("s", holder).ret(ts::void_t()).begin(); {
  auto p = c.structField("s", "p");
  auto pDeref = c.dereferencePointer(p);

  c.writeOut(pDeref);
  c.assign(pDeref, literal::i8('X'));
  c.returnFromFunction();
} c.endFunction();

TEST_END
