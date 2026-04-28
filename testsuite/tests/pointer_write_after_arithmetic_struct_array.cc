// Write through an adjusted pointer into an array of structs
// Expected: AXC

TEST_BEGIN

auto i8 = ts::i8();
auto pairT = ts::defineStruct("Pair")("x", i8,
                                    "y", i8);
auto pairP = ts::pointer(pairT);
auto arrT  = ts::array(pairT, 3);

c.beginFunction("main"); {
  c.declareLocal("p", pairP);
  c.declareLocal("arr", arrT);

  c.beginBlock("entry"); {
    c.assign(c.structField(c.arrayElement("arr", 0), "x"), literal::i8('A'));
    c.assign(c.structField(c.arrayElement("arr", 1), "x"), literal::i8('B'));
    c.assign(c.structField(c.arrayElement("arr", 2), "x"), literal::i8('C'));

    c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
    c.addAssign("p", literal::i16(1));

    auto obj = c.dereferencePointer("p");
    c.assign(c.structField(obj, "x"), literal::i8('X'));

    c.writeOut(c.structField(c.arrayElement("arr", 0), "x")); // A
    c.writeOut(c.structField(c.arrayElement("arr", 1), "x")); // X
    c.writeOut(c.structField(c.arrayElement("arr", 2), "x")); // C

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
