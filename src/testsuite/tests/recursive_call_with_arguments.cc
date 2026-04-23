// Tests recursive self-calls with multiple i8 parameters, stressing repeated frame creation and parameter re-passing.
// Expect: ABCABCABCABC...
TEST_BEGIN
c.beginFunction("main"); {
  c.beginBlock("entry"); {
    auto args = c.constructFunctionArguments(values::i8('A'),
					     values::i8('B'),
					     values::i8('C'));

    c.callFunction("foo", "after_foo", args);
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = TypeSystem::function(TypeSystem::voidT(), TypeSystem::i8(), TypeSystem::i8(), TypeSystem::i8());
c.beginFunction("foo", fooSig, {"x", "y", "z"}); {
  c.beginBlock("entry"); {
    c.writeOut("x");
    c.writeOut("y");
    c.writeOut("z");

    auto args = c.constructFunctionArguments("x", "y", "z");
    c.callFunction("foo", "after_recurse", args);
  } c.endBlock();

  c.beginBlock("after_recurse"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();
TEST_END
