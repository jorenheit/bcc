// Tests repeated pointer-depth adjustment across two call boundaries.
// main creates a pointer to x, foo forwards it to bar, and bar reads/writes through it.
// Expected: AX

TEST_BEGIN

auto i8  = TypeSystem::i8();
auto i8p = TypeSystem::pointer(i8);

c.beginFunction("main"); {
  c.declareLocal("p", i8p);
  c.declareLocal("x", i8);

  c.beginBlock("entry"); {
    c.assign("x", values::i8('A'));
    c.assign("p", values::pointer(i8, "x"));
    auto args = c.constructFunctionArguments("p");
    c.callFunction("foo", "after", args);
  } c.endBlock();

  c.beginBlock("after"); {
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto sig = c.constructFunctionSignature(TypeSystem::voidT(), "p", i8p);
c.beginFunction("foo", sig); {
  c.beginBlock("entry"); {
    auto args = c.constructFunctionArguments("p");
    c.callFunction("bar", "return", args);
  } c.endBlock();

  c.beginBlock("return"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("bar", sig); {
  c.beginBlock("entry"); {
    auto pDeref = c.dereferencePointer("p");
    c.writeOut(pDeref);
    c.assign(pDeref, values::i8('X'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
