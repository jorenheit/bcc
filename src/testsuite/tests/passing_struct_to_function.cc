// Passing entire struct object to function
// Expected: ABABXYAB

TEST_BEGIN

auto pointFields = c.constructFields("x", TypeSystem::i8(),
				     "y", TypeSystem::i8());
auto point = c.defineStruct("Point", pointFields);

c.beginFunction("main"); {
  c.declareLocal("s", point);

  c.beginBlock("entry"); {
    auto x = c.structField("s", "x");
    auto y = c.structField("s", "y");

    c.assign(x, values::i8('A'));
    c.assign(y, values::i8('B'));
    c.writeOut("s");

    auto args = c.constructFunctionArguments("s");
    c.callFunction("foo", "after_foo", args);
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = c.constructFunctionSignature(TypeSystem::voidT(),
					   "p", point);
c.beginFunction("foo", fooSig); {
  c.beginBlock("entry"); {
    auto px = c.structField("p", "x");
    auto py = c.structField("p", "y");

    c.writeOut("p");
    c.assign(px, values::i8('X'));
    c.assign(py, values::i8('Y'));
    c.writeOut("p");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
