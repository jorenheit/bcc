// Write through an adjusted pointer into an i8 array
// Expected: ABXD

TEST_BEGIN

auto i8   = TypeSystem::i8();
auto i8p  = TypeSystem::pointer(i8);
auto arrT = TypeSystem::array(i8, 4);

c.beginFunction("main"); {
  c.declareLocal("p", i8p);
  c.declareLocal("arr", arrT);

  c.beginBlock("entry"); {
    c.assign(c.arrayElement("arr", 0), values::i8('A'));
    c.assign(c.arrayElement("arr", 1), values::i8('B'));
    c.assign(c.arrayElement("arr", 2), values::i8('C'));
    c.assign(c.arrayElement("arr", 3), values::i8('D'));

    c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
    c.addAssign("p", values::i16(2));

    auto pDeref = c.dereferencePointer("p");
    c.assign(pDeref, values::i8('X'));

    c.writeOut(c.arrayElement("arr", 0)); // A
    c.writeOut(c.arrayElement("arr", 1)); // B
    c.writeOut(c.arrayElement("arr", 2)); // X
    c.writeOut(c.arrayElement("arr", 3)); // D

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
