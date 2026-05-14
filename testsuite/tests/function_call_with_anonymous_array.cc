// Tests passing an anonymous array by value to a function
// Expect: ABCD

TEST_BEGIN
auto array4 = ts::array(ts::u8(), 4);

c.function("main").begin(); {
  auto arr = literal::array(ts::array(ts::u8(), 4)).push(literal::u8('A')).push(literal::u8('B')).push(literal::u8('C')).push(literal::u8('D')).done();

  c.callFunction("foo").arg(arr).done();
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("arr", array4).ret(ts::void_t()).begin(); {
  c.write("arr");
  c.returnFromFunction();
} c.endFunction();

TEST_END
