// Do pointer arithmetic on an array of arrays using a variable offset
// Expected: ACB

TEST_BEGIN

auto i8     = TypeSystem::i8();
auto innerT = TypeSystem::array(i8, 2);
auto innerP = TypeSystem::pointer(innerT);
auto outerT = TypeSystem::array(innerT, 3);

c.beginFunction("main"); {
  c.declareLocal("idx", TypeSystem::i16());
  c.declareLocal("p", innerP);
  c.declareLocal("q", innerP);
  c.declareLocal("arr", outerT);

  c.beginBlock("entry"); {
    c.assign(c.arrayElement(c.arrayElement("arr", 0), 0), values::i8('A'));
    c.assign(c.arrayElement(c.arrayElement("arr", 1), 0), values::i8('B'));
    c.assign(c.arrayElement(c.arrayElement("arr", 2), 0), values::i8('C'));

    c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
    auto pArr = c.dereferencePointer("p");
    c.writeOut(c.arrayElement(pArr, 0)); // A

    c.assign("idx", values::i16(2));
    c.assign("q", c.add("p", "idx"));
    auto qArr = c.dereferencePointer("q");
    c.writeOut(c.arrayElement(qArr, 0)); // C

    c.assign("idx", values::i16(1));
    c.subAssign("q", "idx");
    auto qPrev = c.dereferencePointer("q");
    c.writeOut(c.arrayElement(qPrev, 0)); // B

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
