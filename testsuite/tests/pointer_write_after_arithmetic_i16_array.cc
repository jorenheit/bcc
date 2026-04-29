// Write through an adjusted pointer into an i16 array
// Expected: AaXxCc

TEST_BEGIN

auto i16  = ts::i16();
auto i16p = ts::pointer(i16);
auto arrT = ts::array(i16, 3);

c.function("main").begin(); {
  c.declareLocal("p", i16p);
  c.declareLocal("arr", arrT);

  c.block("entry").begin(); {
    c.assign(c.arrayElement("arr", 0), literal::i16(CAT('A', 'a')));
    c.assign(c.arrayElement("arr", 1), literal::i16(CAT('B', 'b')));
    c.assign(c.arrayElement("arr", 2), literal::i16(CAT('C', 'c')));

    c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
    c.addAssign("p", literal::i16(1));

    auto pDeref = c.dereferencePointer("p");
    c.assign(pDeref, literal::i16(CAT('X', 'x')));

    c.writeOut(c.arrayElement("arr", 0)); // Aa
    c.writeOut(c.arrayElement("arr", 1)); // Xx
    c.writeOut(c.arrayElement("arr", 2)); // Cc

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
