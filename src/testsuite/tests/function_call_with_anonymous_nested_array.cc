// Tests passing a nested anonymous array by value to a function
// Expect: ABCD

TEST_BEGIN

auto array2 = TypeSystem::array(TypeSystem::i8(), 2);
auto array22 = TypeSystem::array(array2, 2);

c.beginFunction("main"); {
  c.beginBlock("entry"); {
    auto ab = values::array(TypeSystem::i8(), 'A', 'B');
    auto cd = values::array(TypeSystem::i8(), 'C', 'D');
    auto args = c.constructFunctionArguments(values::array(array2, ab, cd));
    c.callFunction("foo", "after_foo", args);
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = c.constructFunctionSignature(TypeSystem::voidT(),
					   "arr", array22);
c.beginFunction("foo", fooSig); {
  c.beginBlock("entry"); {
    c.writeOut("arr");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
