// Copying a pointer preserves the old target when the original is adjusted
// Expected: AB

TEST_BEGIN

auto u8   = ts::u8();
auto u8p  = ts::pointer(u8);
auto arrT = ts::array(u8, 2);

c.function("main").begin(); {
  c.declareLocal("p", u8p);
  c.declareLocal("q", u8p);
  c.declareLocal("arr", arrT);

  c.assign(c.arrayElement("arr", 0), literal::u8('A'));
  c.assign(c.arrayElement("arr", 1), literal::u8('B'));

  c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
  c.assign("q", "p");

  c.addAssign("p", literal::u16(1));

  c.write(c.dereferencePointer("q")); // A
  c.write(c.dereferencePointer("p")); // B

  c.returnFromFunction();
} c.endFunction();

TEST_END
