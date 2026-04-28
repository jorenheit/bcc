// Tests passing a nested anonymous array by value to a function
// Expect: ABCD

TEST_BEGIN

auto array2 = ts::array(ts::i8(), 2);
auto array22 = ts::array(array2, 2);

c.beginFunction("main"); {
  c.beginBlock("entry"); {
    auto ab = literal::array(ts::i8())(literal::i8('A'), literal::i8('B'));
    auto cd = literal::array(ts::i8())(literal::i8('C'), literal::i8('D'));
			    
    c.callFunction("foo", "after_foo")(literal::array(array2)(ab, cd));
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = ts::function(ts::voidT())(array22);
c.beginFunction("foo", fooSig, {"arr"}); {
  c.beginBlock("entry"); {
    c.writeOut("arr");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
