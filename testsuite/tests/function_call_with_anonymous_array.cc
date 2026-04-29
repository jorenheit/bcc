// Tests passing an anonymous array by value to a function
// Expect: ABCD

TEST_BEGIN
auto array4 = ts::array(ts::i8(), 4);

c.beginFunction("main"); {
  c.beginBlock("entry"); {
    auto arr = literal::array(ts::array(ts::i8(), 4)).push(literal::i8('A')).push(literal::i8('B')).push(literal::i8('C')).push(literal::i8('D')).done();
    
    c.callFunction("foo", "after_foo").arg(arr).done();
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = ts::function().ret(ts::void_t()).param(array4).done();
c.beginFunction("foo", fooSig, {"arr"}); {
  c.beginBlock("entry"); {
    c.writeOut("arr");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
