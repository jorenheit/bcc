// Do pointer arithmetic on an i8 array using add, addAssign and subAssign
// Expected: ACBB

TEST_BEGIN

auto i8   = ts::i8();
auto i8p  = ts::pointer(i8);
auto arrT = ts::array(i8, 4);

c.function("main").begin(); {
  c.declareLocal("p", i8p);
  c.declareLocal("q", i8p);
  c.declareLocal("arr", arrT);

  c.assign(c.arrayElement("arr", 0), literal::i8('A'));
  c.assign(c.arrayElement("arr", 1), literal::i8('B'));
  c.assign(c.arrayElement("arr", 2), literal::i8('C'));
  c.assign(c.arrayElement("arr", 3), literal::i8('D'));

  c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
  c.write(c.dereferencePointer("p"));        // A

  c.assign("q", c.add("p", literal::i16(2)));
  c.write(c.dereferencePointer("q"));        // C

  c.addAssign("p", literal::i16(1));
  c.write(c.dereferencePointer("p"));        // B

  c.subAssign("q", literal::i16(1));
  c.write(c.dereferencePointer("q"));        // B

  c.returnFromFunction();
} c.endFunction();

TEST_END
