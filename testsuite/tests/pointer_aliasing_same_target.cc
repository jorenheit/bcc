// Two pointers alias the same target and observe the same write
// Expected: XX

TEST_BEGIN

auto i8  = TypeSystem::i8();
auto i8p = TypeSystem::pointer(i8);

c.beginFunction("main"); {
  c.declareLocal("x", i8);
  c.declareLocal("p", i8p);
  c.declareLocal("q", i8p);

  c.beginBlock("entry"); {
    c.assign("x", values::i8('A'));
    c.assign("p", c.addressOf("x"));
    c.assign("q", c.addressOf("x"));

    c.assign(c.dereferencePointer("p"), values::i8('X'));

    c.writeOut(c.dereferencePointer("p")); // X
    c.writeOut(c.dereferencePointer("q")); // X

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
