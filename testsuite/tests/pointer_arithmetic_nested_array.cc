// Do pointer arithmetic on an array of arrays and verify scaling by inner array size
// Expected: ACB

TEST_BEGIN

auto u8     = ts::u8();
auto innerT = ts::array(u8, 2);
auto innerP = ts::pointer(innerT);
auto outerT = ts::array(innerT, 3);

c.function("main").begin(); {
  c.declareLocal("p", innerP);
  c.declareLocal("q", innerP);
  c.declareLocal("arr", outerT);

  c.assign(c.arrayElement(c.arrayElement("arr", 0), 0), literal::u8('A'));
  c.assign(c.arrayElement(c.arrayElement("arr", 1), 0), literal::u8('B'));
  c.assign(c.arrayElement(c.arrayElement("arr", 2), 0), literal::u8('C'));

  c.assign("p", c.addressOf(c.arrayElement("arr", 0)));

  auto pArr = c.dereferencePointer("p");
  c.write(c.arrayElement(pArr, 0));         // A

  c.assign("q", c.add("p", literal::u16(2)));
  auto qArr = c.dereferencePointer("q");
  c.write(c.arrayElement(qArr, 0));         // C

  c.subAssign("q", literal::u16(1));
  auto qPrev = c.dereferencePointer("q");
  c.write(c.arrayElement(qPrev, 0));        // B

  c.returnFromFunction();
} c.endFunction();

TEST_END
