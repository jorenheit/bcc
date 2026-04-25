// Tests passing a nested anonymous array by value to a function
// Expect: ABCD

TEST_BEGIN

auto array2 = TypeSystem::array(TypeSystem::i8(), 2);
auto array22 = TypeSystem::array(array2, 2);

c.beginFunction("main"); {
  c.beginBlock("entry"); {
    auto ab = values::array(TypeSystem::i8(), values::i8('A'), values::i8('B'));
    auto cd = values::array(TypeSystem::i8(), values::i8('C'), values::i8('D'));
    c.callFunction("foo", "after_foo")(values::array(array2, ab, cd));
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = TypeSystem::function(TypeSystem::voidT(), array22);
c.beginFunction("foo", fooSig, {"arr"}); {
  c.beginBlock("entry"); {
    c.writeOut("arr");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
