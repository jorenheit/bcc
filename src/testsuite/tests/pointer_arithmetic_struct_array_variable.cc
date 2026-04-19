// Do pointer arithmetic on an array of structs using a variable offset
// Expected: ACB

TEST_BEGIN

auto i8 = TypeSystem::i8();
auto pairFields = c.constructFields("x", i8,
                                    "y", i8);
auto pairT = c.defineStruct("Pair", pairFields);
auto pairP = TypeSystem::pointer(pairT);
auto arrT  = TypeSystem::array(pairT, 3);

c.beginFunction("main"); {
  c.declareLocal("idx", TypeSystem::i16());
  c.declareLocal("p", pairP);
  c.declareLocal("q", pairP);
  c.declareLocal("arr", arrT);

  c.beginBlock("entry"); {
    c.assign(c.structField(c.arrayElement("arr", 0), "x"), values::i8('A'));
    c.assign(c.structField(c.arrayElement("arr", 1), "x"), values::i8('B'));
    c.assign(c.structField(c.arrayElement("arr", 2), "x"), values::i8('C'));

    c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
    auto pObj = c.dereferencePointer("p");
    c.writeOut(c.structField(pObj, "x")); // A

    c.assign("idx", values::i16(2));
    c.assign("q", c.add("p", "idx"));
    auto qObj = c.dereferencePointer("q");
    c.writeOut(c.structField(qObj, "x")); // C

    c.assign("idx", values::i16(1));
    c.subAssign("q", "idx");
    auto qPrev = c.dereferencePointer("q");
    c.writeOut(c.structField(qPrev, "x")); // B

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
