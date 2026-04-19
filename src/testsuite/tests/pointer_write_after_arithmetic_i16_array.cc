// Write through an adjusted pointer into an i16 array
// Expected: AaXxCc

TEST_BEGIN

auto i16  = TypeSystem::i16();
auto i16p = TypeSystem::pointer(i16);
auto arrT = TypeSystem::array(i16, 3);

c.beginFunction("main"); {
  c.declareLocal("p", i16p);
  c.declareLocal("arr", arrT);

  c.beginBlock("entry"); {
    c.assign(c.arrayElement("arr", 0), values::i16(CAT('A', 'a')));
    c.assign(c.arrayElement("arr", 1), values::i16(CAT('B', 'b')));
    c.assign(c.arrayElement("arr", 2), values::i16(CAT('C', 'c')));

    c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
    c.addAssign("p", values::i16(1));

    auto pDeref = c.dereferencePointer("p");
    c.assign(pDeref, values::i16(CAT('X', 'x')));

    c.writeOut(c.arrayElement("arr", 0)); // Aa
    c.writeOut(c.arrayElement("arr", 1)); // Xx
    c.writeOut(c.arrayElement("arr", 2)); // Cc

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
