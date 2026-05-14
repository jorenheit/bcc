// Tests reading through an array of pointers passed to a function.
// foo reads both pointees, main then prints the original locals.
// Expected: ABAB

TEST_BEGIN

auto u8   = ts::u8();
auto u8p  = ts::pointer(u8);
auto u8pa = ts::array(u8p, 2);

c.function("main").begin(); {
  c.declareLocal("p", u8pa);
  c.declareLocal("a", u8);
  c.declareLocal("b", u8);

  c.assign("a", literal::u8('A'));
  c.assign("b", literal::u8('B'));

  c.assign(c.arrayElement("p", 0), c.addressOf("a"));
  c.assign(c.arrayElement("p", 1), c.addressOf("b"));

  c.callFunction("foo").arg("p").done();
  c.write("a");
  c.write("b");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("p", u8pa).ret(ts::void_t()).begin(); {
  auto p0Deref = c.dereferencePointer(c.arrayElement("p", 0));
  auto p1Deref = c.dereferencePointer(c.arrayElement("p", 1));

  c.write(p0Deref);
  c.write(p1Deref);
  c.returnFromFunction();
} c.endFunction();

TEST_END
