// Tests a struct containing a pointer field passed to a function.
// foo reads through s.p, then writes through it. main observes the updated pointee.
// Expected: AAX

TEST_BEGIN

auto i8  = TypeSystem::i8();
auto i8p = TypeSystem::pointer(i8);

auto holderFields = c.constructFields("p", i8p);
auto holder = c.defineStruct("Holder", holderFields);

c.beginFunction("main"); {
  c.declareLocal("s", holder);
  c.declareLocal("x", i8);

  c.beginBlock("entry"); {
    c.assign("x", values::i8('A'));
    c.assign(c.structField("s", "p"), c.addressOf("x"));

    auto args = c.constructFunctionArguments("s");
    c.callFunction("foo", "after", args);
  } c.endBlock();

  c.beginBlock("after"); {
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto sig = c.constructFunctionSignature(TypeSystem::voidT(),
					"s", holder);
c.beginFunction("foo", sig); {
  c.beginBlock("entry"); {
    auto p = c.structField("s", "p");
    auto pDeref = c.dereferencePointer(p);

    c.writeOut(pDeref);
    c.assign(pDeref, values::i8('X'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
