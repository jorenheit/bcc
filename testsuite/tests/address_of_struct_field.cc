// Take the address of a struct field and read/write through it
// Expected: BAX

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);

auto pairT = ts::defineStruct("Pair").field("x", i8).field("y", i8).done();

c.function("main").begin(); {
  c.declareLocal("p", i8p);
  c.declareLocal("s", pairT);

  c.block("entry").begin(); {
    c.assign(c.structField("s", "x"), literal::i8('A'));
    c.assign(c.structField("s", "y"), literal::i8('B'));

    c.assign("p", c.addressOf(c.structField("s", "y")));

    auto pDeref = c.dereferencePointer("p");
    c.writeOut(pDeref);                  // B

    c.assign(pDeref, literal::i8('X'));

    c.writeOut(c.structField("s", "x")); // A
    c.writeOut(c.structField("s", "y")); // X

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
