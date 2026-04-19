// Take the address of a struct field and read/write through it
// Expected: BAX

TEST_BEGIN

auto i8  = TypeSystem::i8();
auto i8p = TypeSystem::pointer(i8);

auto pairFields = c.constructFields("x", i8,
                                    "y", i8);
auto pairT = c.defineStruct("Pair", pairFields);

c.beginFunction("main"); {
  c.declareLocal("p", i8p);
  c.declareLocal("s", pairT);

  c.beginBlock("entry"); {
    c.assign(c.structField("s", "x"), values::i8('A'));
    c.assign(c.structField("s", "y"), values::i8('B'));

    c.assign("p", c.addressOf(c.structField("s", "y")));

    auto pDeref = c.dereferencePointer("p");
    c.writeOut(pDeref);                  // B

    c.assign(pDeref, values::i8('X'));

    c.writeOut(c.structField("s", "x")); // A
    c.writeOut(c.structField("s", "y")); // X

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
