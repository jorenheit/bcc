// Tests a struct containing a pointer field passed to a function.
// foo reads through s.p, then writes through it. main observes the updated pointee.
// Expected: AAX

TEST_BEGIN

auto u8  = ts::u8();
auto u8p = ts::pointer(u8);

auto holder = ts::defineStruct("Holder").field("p", u8p).done();

c.function("main").begin(); {
  c.declareLocal("s", holder);
  c.declareLocal("x", u8);

  c.assign("x", literal::u8('A'));
  c.assign(c.structField("s", "p"), c.addressOf("x"));

  c.callFunction("foo").arg("s").done();
  c.write("x");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("s", holder).ret(ts::void_t()).begin(); {
  auto p = c.structField("s", "p");
  auto pDeref = c.dereferencePointer(p);

  c.write(pDeref);
  c.assign(pDeref, literal::u8('X'));
  c.returnFromFunction();
} c.endFunction();

TEST_END
