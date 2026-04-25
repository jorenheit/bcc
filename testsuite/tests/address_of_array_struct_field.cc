// Take the address of a struct field inside an array element and read/write through it
// Expected: baXc

TEST_BEGIN

auto i8  = TypeSystem::i8();
auto i8p = TypeSystem::pointer(i8);

auto pairT = c.defineStruct("Pair")("x", i8,
                                    "y", i8);
auto arrT = TypeSystem::array(pairT, 3);

c.beginFunction("main"); {
  c.declareLocal("p", i8p);
  c.declareLocal("arr", arrT);

  c.beginBlock("entry"); {
    c.assign(c.structField(c.arrayElement("arr", 0), "y"), values::i8('a'));
    c.assign(c.structField(c.arrayElement("arr", 1), "y"), values::i8('b'));
    c.assign(c.structField(c.arrayElement("arr", 2), "y"), values::i8('c'));

    c.assign("p", c.addressOf(c.structField(c.arrayElement("arr", 1), "y")));

    auto pDeref = c.dereferencePointer("p");
    c.writeOut(pDeref);                                    // b

    c.assign(pDeref, values::i8('X'));

    c.writeOut(c.structField(c.arrayElement("arr", 0), "y")); // a
    c.writeOut(c.structField(c.arrayElement("arr", 1), "y")); // X
    c.writeOut(c.structField(c.arrayElement("arr", 2), "y")); // c

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
