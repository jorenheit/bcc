// Two pointers alias the same target and observe the same write
// Expected: XX

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);

c.function("main").begin(); {
  c.declareLocal("x", i8);
  c.declareLocal("p", i8p);
  c.declareLocal("q", i8p);

  c.block("entry").begin(); {
    c.assign("x", literal::i8('A'));
    c.assign("p", c.addressOf("x"));
    c.assign("q", c.addressOf("x"));

    c.assign(c.dereferencePointer("p"), literal::i8('X'));

    c.writeOut(c.dereferencePointer("p")); // X
    c.writeOut(c.dereferencePointer("q")); // X

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
