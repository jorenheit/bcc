// Return a value from a function directly into an array element selected by a runtime index.
// Expected: "ABCZ"

TEST_BEGIN

auto array4 = ts::array(ts::i8(), 4);

c.function("main").begin(); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", ts::i8());

  c.assign("arr", literal::array(ts::array(ts::i8(), 4)).push(literal::i8('A')).push(literal::i8('B')).push(literal::i8('C')).push(literal::i8('D')).done());
  c.assign("idx", literal::i8(3));

  auto dest = c.arrayElement("arr", "idx");
  c.callFunction("makeZ").into(dest).done();
  c.write("arr");
  c.returnFromFunction();
} c.endFunction();

c.function("makeZ").ret(ts::i8()).begin(); {
  c.returnFromFunction(literal::i8('Z'));
} c.endFunction();

TEST_END
