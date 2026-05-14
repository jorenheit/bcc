// Tests recursive pointer relocation through array -> struct -> pointer.
// foo reads through both pointer fields, then writes through both.
// Expected: ABXY

TEST_BEGIN

auto u8  = ts::u8();
auto u8p = ts::pointer(u8);

auto holder = ts::defineStruct("Holder").field("p", u8p).done();
auto holders = ts::array(holder, 2);

c.function("main").begin(); {
  c.declareLocal("arr", holders);
  c.declareLocal("a", u8);
  c.declareLocal("b", u8);

  c.assign("a", literal::u8('A'));
  c.assign("b", literal::u8('B'));

  c.assign(c.structField(c.arrayElement("arr", 0), "p"), c.addressOf("a"));
  c.assign(c.structField(c.arrayElement("arr", 1), "p"), c.addressOf("b"));

  c.callFunction("foo").arg("arr").done();
  c.write("a");
  c.write("b");
  c.returnFromFunction();
} c.endFunction();


c.function("foo").param("arr", holders).ret(ts::void_t()).begin(); {
  auto p0 = c.structField(c.arrayElement("arr", 0), "p");
  auto p1 = c.structField(c.arrayElement("arr", 1), "p");

  auto d0 = c.dereferencePointer(p0);
  auto d1 = c.dereferencePointer(p1);

  c.write(d0);
  c.write(d1);

  c.assign(d0, literal::u8('X'));
  c.assign(d1, literal::u8('Y'));

  c.returnFromFunction();
} c.endFunction();

TEST_END
