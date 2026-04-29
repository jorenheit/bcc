// Tests passing a nested anonymous array by value to a function
// Expect: ABCD

TEST_BEGIN

auto array2 = ts::array(ts::i8(), 2);
auto array22 = ts::array(array2, 2);

c.beginFunction("main"); {
  c.beginBlock("entry"); {
    auto ab = literal::array(ts::array(ts::i8(), 2)).push(literal::i8('A')).push(literal::i8('B')).done();
    auto cd = literal::array(ts::array(ts::i8(), 2)).push(literal::i8('C')).push(literal::i8('D')).done();
			    
    c.callFunction("foo", "after_foo").arg(literal::array(ts::array(array2, 2)).push(ab).push(cd).done()).done();
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = ts::function().ret(ts::void_t()).param(array22).done();
c.beginFunction("foo", fooSig, {"arr"}); {
  c.beginBlock("entry"); {
    c.writeOut("arr");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
