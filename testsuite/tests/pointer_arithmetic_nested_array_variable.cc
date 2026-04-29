// Do pointer arithmetic on an array of arrays using a variable offset
// Expected: ACB

TEST_BEGIN

auto i8     = ts::i8();
auto innerT = ts::array(i8, 2);
auto innerP = ts::pointer(innerT);
auto outerT = ts::array(innerT, 3);

c.function("main").begin(); {
  c.declareLocal("idx", ts::i16());
  c.declareLocal("p", innerP);
  c.declareLocal("q", innerP);
  c.declareLocal("arr", outerT);

  c.block("entry").begin(); {
    c.assign(c.arrayElement(c.arrayElement("arr", 0), 0), literal::i8('A'));
    c.assign(c.arrayElement(c.arrayElement("arr", 1), 0), literal::i8('B'));
    c.assign(c.arrayElement(c.arrayElement("arr", 2), 0), literal::i8('C'));

    c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
    auto pArr = c.dereferencePointer("p");
    c.writeOut(c.arrayElement(pArr, 0)); // A

    c.assign("idx", literal::i16(2));
    c.assign("q", c.add("p", "idx"));
    auto qArr = c.dereferencePointer("q");
    c.writeOut(c.arrayElement(qArr, 0)); // C

    c.assign("idx", literal::i16(1));
    c.subAssign("q", "idx");
    auto qPrev = c.dereferencePointer("q");
    c.writeOut(c.arrayElement(qPrev, 0)); // B

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
