// Write through an adjusted pointer into an array of structs
// Expected: AXC

TEST_BEGIN

auto i8 = TypeSystem::i8();
auto pairFields = c.constructFields("x", i8,
                                    "y", i8);
auto pairT = c.defineStruct("Pair", pairFields);
auto pairP = TypeSystem::pointer(pairT);
auto arrT  = TypeSystem::array(pairT, 3);

c.beginFunction("main"); {
  c.declareLocal("p", pairP);
  c.declareLocal("arr", arrT);

  c.beginBlock("entry"); {
    c.assign(c.structField(c.arrayElement("arr", 0), "x"), values::i8('A'));
    c.assign(c.structField(c.arrayElement("arr", 1), "x"), values::i8('B'));
    c.assign(c.structField(c.arrayElement("arr", 2), "x"), values::i8('C'));

    c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
    c.addAssign("p", values::i16(1));

    auto obj = c.dereferencePointer("p");
    c.assign(c.structField(obj, "x"), values::i8('X'));

    c.writeOut(c.structField(c.arrayElement("arr", 0), "x")); // A
    c.writeOut(c.structField(c.arrayElement("arr", 1), "x")); // X
    c.writeOut(c.structField(c.arrayElement("arr", 2), "x")); // C

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
