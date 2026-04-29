// Do pointer arithmetic on an array of structs using a variable offset
// Expected: ACB

TEST_BEGIN

auto i8 = ts::i8();
auto pairT = ts::defineStruct("Pair").field("x", i8).field("y", i8).done();
auto pairP = ts::pointer(pairT);
auto arrT  = ts::array(pairT, 3);

c.function("main").begin(); {
  c.declareLocal("idx", ts::i16());
  c.declareLocal("p", pairP);
  c.declareLocal("q", pairP);
  c.declareLocal("arr", arrT);

  c.block("entry").begin(); {
    c.assign(c.structField(c.arrayElement("arr", 0), "x"), literal::i8('A'));
    c.assign(c.structField(c.arrayElement("arr", 1), "x"), literal::i8('B'));
    c.assign(c.structField(c.arrayElement("arr", 2), "x"), literal::i8('C'));

    c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
    auto pObj = c.dereferencePointer("p");
    c.writeOut(c.structField(pObj, "x")); // A

    c.assign("idx", literal::i16(2));
    c.assign("q", c.add("p", "idx"));
    auto qObj = c.dereferencePointer("q");
    c.writeOut(c.structField(qObj, "x")); // C

    c.assign("idx", literal::i16(1));
    c.subAssign("q", "idx");
    auto qPrev = c.dereferencePointer("q");
    c.writeOut(c.structField(qPrev, "x")); // B

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
