// Tests reading through a pointer to a global variable in the same function.
// Expected: GG

TEST_BEGIN

auto i8  = TypeSystem::i8();
auto i8p = TypeSystem::pointer(i8);

c.declareGlobal("g", i8);

c.beginFunction("main"); {
  c.referGlobals({"g"});
  c.declareLocal("p", i8p);

  c.beginBlock("entry"); {
    c.assign("g", values::i8('G'));
    c.assign("p", values::pointer(i8, "g"));

    auto pDeref = c.dereferencePointer("p");
    c.writeOut("g");
    c.writeOut(pDeref);

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
