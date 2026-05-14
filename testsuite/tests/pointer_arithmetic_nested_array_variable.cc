// Do pointer arithmetic on an array of arrays using a variable offset
// Expected: ACB

TEST_BEGIN

auto u8     = ts::u8();
auto innerT = ts::array(u8, 2);
auto innerP = ts::pointer(innerT);
auto outerT = ts::array(innerT, 3);

c.function("main").begin(); {
  c.declareLocal("idx", ts::u16());
  c.declareLocal("p", innerP);
  c.declareLocal("q", innerP);
  c.declareLocal("arr", outerT);

  c.assign(c.arrayElement(c.arrayElement("arr", 0), 0), literal::u8('A'));
  c.assign(c.arrayElement(c.arrayElement("arr", 1), 0), literal::u8('B'));
  c.assign(c.arrayElement(c.arrayElement("arr", 2), 0), literal::u8('C'));

  c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
  auto pArr = c.dereferencePointer("p");
  c.write(c.arrayElement(pArr, 0)); // A

  c.assign("idx", literal::u16(2));
  c.assign("q", c.add("p", "idx"));
  auto qArr = c.dereferencePointer("q");
  c.write(c.arrayElement(qArr, 0)); // C

  c.assign("idx", literal::u16(1));
  c.subAssign("q", "idx");
  auto qPrev = c.dereferencePointer("q");
  c.write(c.arrayElement(qPrev, 0)); // B

  c.returnFromFunction();
} c.endFunction();

TEST_END
