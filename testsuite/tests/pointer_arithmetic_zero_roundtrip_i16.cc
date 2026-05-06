// Pointer arithmetic with zero and round-trip movement on i16 elements
// Expected: AaAaAa

TEST_BEGIN

auto i16  = ts::i16();
auto i16p = ts::pointer(i16);
auto arrT = ts::array(i16, 3);

c.function("main").begin(); {
  c.declareLocal("p", i16p);
  c.declareLocal("q", i16p);
  c.declareLocal("arr", arrT);

  c.assign(c.arrayElement("arr", 0), literal::i16(CAT('A', 'a')));
  c.assign(c.arrayElement("arr", 1), literal::i16(CAT('B', 'b')));
  c.assign(c.arrayElement("arr", 2), literal::i16(CAT('C', 'c')));

  c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
  c.writeOut(c.dereferencePointer("p")); // Aa

  c.assign("q", c.add("p", literal::i16(0)));
  c.writeOut(c.dereferencePointer("q")); // Aa

  c.addAssign("p", literal::i16(2));
  c.subAssign("p", literal::i16(2));
  c.writeOut(c.dereferencePointer("p")); // Aa

  c.returnFromFunction();
} c.endFunction();

TEST_END
