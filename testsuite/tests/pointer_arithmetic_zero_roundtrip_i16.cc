// Pointer arithmetic with zero and round-trip movement on u16 elements
// Expected: AaAaAa

TEST_BEGIN

auto u16  = ts::u16();
auto u16p = ts::pointer(u16);
auto arrT = ts::array(u16, 3);

c.function("main").begin(); {
  c.declareLocal("p", u16p);
  c.declareLocal("q", u16p);
  c.declareLocal("arr", arrT);

  c.assign(c.arrayElement("arr", 0), literal::u16(CAT('A', 'a')));
  c.assign(c.arrayElement("arr", 1), literal::u16(CAT('B', 'b')));
  c.assign(c.arrayElement("arr", 2), literal::u16(CAT('C', 'c')));

  c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
  c.write(c.dereferencePointer("p")); // Aa

  c.assign("q", c.add("p", literal::u16(0)));
  c.write(c.dereferencePointer("q")); // Aa

  c.addAssign("p", literal::u16(2));
  c.subAssign("p", literal::u16(2));
  c.write(c.dereferencePointer("p")); // Aa

  c.returnFromFunction();
} c.endFunction();

TEST_END
