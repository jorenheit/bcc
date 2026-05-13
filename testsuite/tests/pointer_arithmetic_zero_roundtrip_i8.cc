// Pointer arithmetic with zero and round-trip movement on i8 elements
// Expected: AAA

TEST_BEGIN

auto i8   = ts::i8();
auto i8p  = ts::pointer(i8);
auto arrT = ts::array(i8, 3);

c.function("main").begin(); {
  c.declareLocal("p", i8p);
  c.declareLocal("q", i8p);
  c.declareLocal("arr", arrT);

  c.assign(c.arrayElement("arr", 0), literal::i8('A'));
  c.assign(c.arrayElement("arr", 1), literal::i8('B'));
  c.assign(c.arrayElement("arr", 2), literal::i8('C'));

  c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
  c.write(c.dereferencePointer("p")); // A

  c.assign("q", c.add("p", literal::i16(0)));
  c.write(c.dereferencePointer("q")); // A

  c.addAssign("p", literal::i16(2));
  c.subAssign("p", literal::i16(2));
  c.write(c.dereferencePointer("p")); // A

  c.returnFromFunction();
} c.endFunction();

TEST_END
