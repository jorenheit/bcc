// Tests writing through a pointer to a global variable from inside a function.
// main initializes g to 'G', foo writes 'H' through p, then main prints g.
// Expected: HH

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
    c.callFunction("foo", "after", "p");
  } c.endBlock();

  c.beginBlock("after"); {
    c.writeOut("g");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("foo", TypeSystem::voidT(), "p", i8p); {
  c.referGlobals({"g"});
  c.beginBlock("entry"); {
    auto pDeref = c.dereferencePointer("p");
    c.assign(pDeref, values::i8('H'));
    c.writeOut("g");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
