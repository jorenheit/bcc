// Pointer arithmetic with zero and round-trip movement on i16 elements
// Expected: AaAaAa

TEST_BEGIN

auto i16  = TypeSystem::i16();
auto i16p = TypeSystem::pointer(i16);
auto arrT = TypeSystem::array(i16, 3);

c.beginFunction("main"); {
  c.declareLocal("p", i16p);
  c.declareLocal("q", i16p);
  c.declareLocal("arr", arrT);

  c.beginBlock("entry"); {
    c.assign(c.arrayElement("arr", 0), values::i16(CAT('A', 'a')));
    c.assign(c.arrayElement("arr", 1), values::i16(CAT('B', 'b')));
    c.assign(c.arrayElement("arr", 2), values::i16(CAT('C', 'c')));

    c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
    c.writeOut(c.dereferencePointer("p")); // Aa

    c.assign("q", c.add("p", values::i16(0)));
    c.writeOut(c.dereferencePointer("q")); // Aa

    c.addAssign("p", values::i16(2));
    c.subAssign("p", values::i16(2));
    c.writeOut(c.dereferencePointer("p")); // Aa

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
