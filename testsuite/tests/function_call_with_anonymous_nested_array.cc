// Tests passing a nested anonymous array by value to a function
// Expect: ABCD

TEST_BEGIN

auto array2 = ts::array(ts::i8(), 2);
auto array22 = ts::array(array2, 2);

c.function("main").begin(); {
  auto ab = literal::array(ts::array(ts::i8(), 2)).push(literal::i8('A')).push(literal::i8('B')).done();
  auto cd = literal::array(ts::array(ts::i8(), 2)).push(literal::i8('C')).push(literal::i8('D')).done();

  c.callFunction("foo").arg(literal::array(ts::array(array2, 2)).push(ab).push(cd).done()).done();
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("arr", array22).ret(ts::void_t()).begin(); {
  c.writeOut("arr");
  c.returnFromFunction();
} c.endFunction();

TEST_END
