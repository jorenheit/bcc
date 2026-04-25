// Tests writing through a pointer to a global variable in the same function.
// After writing through p, reading g by name should observe the update.
// Expected: GH

TEST_BEGIN

auto i8  = TypeSystem::i8();
auto i8p = TypeSystem::pointer(i8);

c.declareGlobal("g", i8);

c.beginFunction("main"); {
  c.referGlobals({"g"});
  c.declareLocal("p", i8p);

  c.beginBlock("entry"); {
    c.assign("g", values::i8('G'));
    c.assign("p", c.addressOf("g"));

    auto pDeref = c.dereferencePointer("p");
    c.writeOut("g");
    c.assign(pDeref, values::i8('H'));
    c.writeOut("g");

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
