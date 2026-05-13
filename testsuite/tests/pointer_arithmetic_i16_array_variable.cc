// Do pointer arithmetic on an i16 array using a variable offset
// Expected: AaCcBb

TEST_BEGIN

auto i16  = ts::i16();
auto i16p = ts::pointer(i16);
auto arrT = ts::array(i16, 3);

c.function("main").begin(); {
  c.declareLocal("idx", i16);
  c.declareLocal("p", i16p);
  c.declareLocal("q", i16p);
  c.declareLocal("arr", arrT);

  c.assign(c.arrayElement("arr", 0), literal::i16(CAT('A', 'a')));
  c.assign(c.arrayElement("arr", 1), literal::i16(CAT('B', 'b')));
  c.assign(c.arrayElement("arr", 2), literal::i16(CAT('C', 'c')));

  c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
  c.write(c.dereferencePointer("p"));        // Aa

  c.assign("idx", literal::i16(2));
  c.assign("q", c.add("p", "idx"));
  c.write(c.dereferencePointer("q"));        // Cc

  c.assign("idx", literal::i16(1));
  c.subAssign("q", "idx");
  c.write(c.dereferencePointer("q"));        // Bb

  c.returnFromFunction();
} c.endFunction();

TEST_END
