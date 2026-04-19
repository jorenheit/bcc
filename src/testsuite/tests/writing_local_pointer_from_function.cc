// Tests writing through a pointer that points to a caller local and is passed to a function.
// foo writes 'Y' through p, so x changes from 'X' to 'Y'.
// Expected: YY

TEST_BEGIN

auto i8  = TypeSystem::i8();
auto i8p = TypeSystem::pointer(i8);

c.beginFunction("main"); {
  c.declareLocal("p", i8p);
  c.declareLocal("x", i8);

  c.beginBlock("entry"); {
    c.assign("x", values::i8('X'));
    c.assign("p", c.addressOf("x"));
    auto args = c.constructFunctionArguments("p");
    c.callFunction("foo", "after", args);
  } c.endBlock();

  c.beginBlock("after"); {
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto sig = c.constructFunctionSignature(TypeSystem::voidT(),
			     "p", i8p);

c.beginFunction("foo", sig); {
  c.beginBlock("entry"); {
    auto pDeref = c.dereferencePointer("p");
    c.assign(pDeref, values::i8('Y'));
    c.writeOut(pDeref);
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
