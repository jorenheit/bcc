// Pass a runtime-selected array element as a function argument and print it inside the callee.
// Expected: "C"

TEST_BEGIN

auto array4 = ts::array(ts::u8(), 4);

c.function("main").begin(); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", ts::u8());

  c.assign("arr", literal::array(ts::array(ts::u8(), 4)).push(literal::u8('A')).push(literal::u8('B')).push(literal::u8('C')).push(literal::u8('D')).done());
  c.assign("idx", literal::u8(2));

  auto elem = c.arrayElement("arr", "idx");
  c.callFunction("printChar").arg(elem).done();
  c.returnFromFunction();
} c.endFunction();

c.function("printChar").param("ch", ts::u8()).ret(ts::void_t()).begin(); {
  c.write("ch");
  c.returnFromFunction();
} c.endFunction();

TEST_END
