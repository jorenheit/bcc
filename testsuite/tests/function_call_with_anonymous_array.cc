// Tests passing an anonymous array by value to a function
// Expect: ABCD

TEST_BEGIN
auto array4 = ts::array(ts::i8(), 4);

c.beginFunction("main"); {
  c.beginBlock("entry"); {
    auto arr = literal::array(ts::i8())(literal::i8('A'),
					       literal::i8('B'),
					       literal::i8('C'),
					       literal::i8('D'));
    
    c.callFunction("foo", "after_foo")(arr);
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = ts::function(ts::voidT())(array4);
c.beginFunction("foo", fooSig, {"arr"}); {
  c.beginBlock("entry"); {
    c.writeOut("arr");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
