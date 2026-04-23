// Tests forwarding an i8 parameter through a nested call chain (main -> foo -> bar).
// Expect: A

TEST_BEGIN
c.beginFunction("main"); {
  c.beginBlock("entry"); {
    auto args = c.constructFunctionArguments(values::i8('A'));
    c.callFunction("foo", "after_foo", args);
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = TypeSystem::function(TypeSystem::voidT(), TypeSystem::i8());
c.beginFunction("foo", fooSig, {"x"}); {
  c.beginBlock("entry"); {
    auto args = c.constructFunctionArguments("x");
    c.callFunction("bar", "after_bar", args);
  } c.endBlock();

  c.beginBlock("after_bar"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto barSig = TypeSystem::function(TypeSystem::voidT(), TypeSystem::i8());
c.beginFunction("bar", barSig, {"y"}); {
  c.beginBlock("entry"); {
    c.writeOut("y");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
