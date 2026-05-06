// Tests passing an anonymous array by value to a function
// Expect: ABCD

TEST_BEGIN
auto array4 = ts::array(ts::i8(), 4);

c.function("main").begin(); {
  auto arr = literal::array(ts::array(ts::i8(), 4)).push(literal::i8('A')).push(literal::i8('B')).push(literal::i8('C')).push(literal::i8('D')).done();

  c.callFunction("foo").arg(arr).done();
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("arr", array4).ret(ts::void_t()).begin(); {
  c.writeOut("arr");
  c.returnFromFunction();
} c.endFunction();

TEST_END
