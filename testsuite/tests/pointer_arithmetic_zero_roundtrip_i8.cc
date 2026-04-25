// Pointer arithmetic with zero and round-trip movement on i8 elements
// Expected: AAA

TEST_BEGIN

auto i8   = TypeSystem::i8();
auto i8p  = TypeSystem::pointer(i8);
auto arrT = TypeSystem::array(i8, 3);

c.beginFunction("main"); {
  c.declareLocal("p", i8p);
  c.declareLocal("q", i8p);
  c.declareLocal("arr", arrT);

  c.beginBlock("entry"); {
    c.assign(c.arrayElement("arr", 0), values::i8('A'));
    c.assign(c.arrayElement("arr", 1), values::i8('B'));
    c.assign(c.arrayElement("arr", 2), values::i8('C'));

    c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
    c.writeOut(c.dereferencePointer("p")); // A

    c.assign("q", c.add("p", values::i16(0)));
    c.writeOut(c.dereferencePointer("q")); // A

    c.addAssign("p", values::i16(2));
    c.subAssign("p", values::i16(2));
    c.writeOut(c.dereferencePointer("p")); // A

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
