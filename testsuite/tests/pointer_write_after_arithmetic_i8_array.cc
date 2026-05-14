// Write through an adjusted pointer into an u8 array
// Expected: ABXD

TEST_BEGIN

auto u8   = ts::u8();
auto u8p  = ts::pointer(u8);
auto arrT = ts::array(u8, 4);

c.function("main").begin(); {
  c.declareLocal("p", u8p);
  c.declareLocal("arr", arrT);

  c.assign(c.arrayElement("arr", 0), literal::u8('A'));
  c.assign(c.arrayElement("arr", 1), literal::u8('B'));
  c.assign(c.arrayElement("arr", 2), literal::u8('C'));
  c.assign(c.arrayElement("arr", 3), literal::u8('D'));

  c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
  c.addAssign("p", literal::u16(2));

  auto pDeref = c.dereferencePointer("p");
  c.assign(pDeref, literal::u8('X'));

  c.write(c.arrayElement("arr", 0)); // A
  c.write(c.arrayElement("arr", 1)); // B
  c.write(c.arrayElement("arr", 2)); // X
  c.write(c.arrayElement("arr", 3)); // D

  c.returnFromFunction();
} c.endFunction();

TEST_END
