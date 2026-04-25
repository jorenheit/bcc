// Tests pass-by-value of an array of i16 values, verifying multi-cell argument copying across a function call.
// Expect: ABCDABCD

TEST_BEGIN
auto array2 = TypeSystem::array(TypeSystem::i16(), 2);

c.beginFunction("main"); {
  c.declareLocal("x", array2);

  c.beginBlock("entry"); {
    auto x0 = c.arrayElement("x", 0);
    auto x1 = c.arrayElement("x", 1);

    c.assign(x0, values::i16(CAT('A', 'B')));
    c.assign(x1, values::i16(CAT('C', 'D')));

    c.writeOut("x");
    auto args = c.constructFunctionArguments("x");
    c.callFunction("foo", "after_foo", args);
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = TypeSystem::function(TypeSystem::voidT(), array2);
c.beginFunction("foo", fooSig, {"x"}); {
  c.beginBlock("entry"); {
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
