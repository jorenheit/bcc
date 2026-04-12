// Tests reading through a pointer that points to a caller local and is passed to a function.
// Expected: XX

TEST_BEGIN

auto i8  = TypeSystem::i8();
auto i8p = TypeSystem::pointer(i8);

c.beginFunction("main"); {
  c.declareLocal("p", i8p);
  c.declareLocal("x", i8);

  c.beginBlock("entry"); {
    c.assign("x", values::i8('X'));
    c.assign("p", values::pointer(i8, "x"));
    c.callFunction("foo", "after", "p");
  } c.endBlock();

  c.beginBlock("after"); {
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("foo", TypeSystem::voidT(), "p", i8p); {
  c.beginBlock("entry"); {
    auto pDeref = c.dereferencePointer("p");
    c.writeOut(pDeref);
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
