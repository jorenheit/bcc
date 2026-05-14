// Do pointer arithmetic on an u16 array using a variable offset
// Expected: AaCcBb

TEST_BEGIN

auto u16  = ts::u16();
auto u16p = ts::pointer(u16);
auto arrT = ts::array(u16, 3);

c.function("main").begin(); {
  c.declareLocal("idx", u16);
  c.declareLocal("p", u16p);
  c.declareLocal("q", u16p);
  c.declareLocal("arr", arrT);

  c.assign(c.arrayElement("arr", 0), literal::u16(CAT('A', 'a')));
  c.assign(c.arrayElement("arr", 1), literal::u16(CAT('B', 'b')));
  c.assign(c.arrayElement("arr", 2), literal::u16(CAT('C', 'c')));

  c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
  c.write(c.dereferencePointer("p"));        // Aa

  c.assign("idx", literal::u16(2));
  c.assign("q", c.add("p", "idx"));
  c.write(c.dereferencePointer("q"));        // Cc

  c.assign("idx", literal::u16(1));
  c.subAssign("q", "idx");
  c.write(c.dereferencePointer("q"));        // Bb

  c.returnFromFunction();
} c.endFunction();

TEST_END
