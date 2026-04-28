// Tests reading through a local pointer and then writing back through it in the same frame.
// x is initialized to 'X', y receives *p, then *p is assigned 'Y'.
// Expected: XXYX

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);

c.beginFunction("main"); {
  c.declareLocal("p", i8p);
  c.declareLocal("x", i8);
  c.declareLocal("y", i8);

  c.beginBlock("entry"); {
    c.assign("p", c.addressOf("x")); //literal::pointer(i8, "x"));
    c.assign("x", literal::i8('X'));

    auto pDeref = c.dereferencePointer("p");
    c.assign("y", pDeref);

    c.writeOut("x");
    c.writeOut("y");

    c.assign(pDeref, literal::i8('Y'));

    c.writeOut("x");
    c.writeOut("y");

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
