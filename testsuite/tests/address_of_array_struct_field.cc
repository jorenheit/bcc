// Take the address of a struct field inside an array element and read/write through it
// Expected: baXc

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);

auto pairT = ts::defineStruct("Pair")("x", i8,
                                    "y", i8);
auto arrT = ts::array(pairT, 3);

c.beginFunction("main"); {
  c.declareLocal("p", i8p);
  c.declareLocal("arr", arrT);

  c.beginBlock("entry"); {
    c.assign(c.structField(c.arrayElement("arr", 0), "y"), literal::i8('a'));
    c.assign(c.structField(c.arrayElement("arr", 1), "y"), literal::i8('b'));
    c.assign(c.structField(c.arrayElement("arr", 2), "y"), literal::i8('c'));

    c.assign("p", c.addressOf(c.structField(c.arrayElement("arr", 1), "y")));

    auto pDeref = c.dereferencePointer("p");
    c.writeOut(pDeref);                                    // b

    c.assign(pDeref, literal::i8('X'));

    c.writeOut(c.structField(c.arrayElement("arr", 0), "y")); // a
    c.writeOut(c.structField(c.arrayElement("arr", 1), "y")); // X
    c.writeOut(c.structField(c.arrayElement("arr", 2), "y")); // c

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
