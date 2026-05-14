// Write through an adjusted pointer into an u16 array
// Expected: AaXxCc

TEST_BEGIN

auto u16  = ts::u16();
auto u16p = ts::pointer(u16);
auto arrT = ts::array(u16, 3);

c.function("main").begin(); {
  c.declareLocal("p", u16p);
  c.declareLocal("arr", arrT);

  c.assign(c.arrayElement("arr", 0), literal::u16(CAT('A', 'a')));
  c.assign(c.arrayElement("arr", 1), literal::u16(CAT('B', 'b')));
  c.assign(c.arrayElement("arr", 2), literal::u16(CAT('C', 'c')));

  c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
  c.addAssign("p", literal::u16(1));

  auto pDeref = c.dereferencePointer("p");
  c.assign(pDeref, literal::u16(CAT('X', 'x')));

  c.write(c.arrayElement("arr", 0)); // Aa
  c.write(c.arrayElement("arr", 1)); // Xx
  c.write(c.arrayElement("arr", 2)); // Cc

  c.returnFromFunction();
} c.endFunction();

TEST_END
