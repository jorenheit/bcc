// Return a value from a function directly into an array element selected by a runtime index.
// Expected: "ABCZ"

TEST_BEGIN

auto array4 = ts::array(ts::i8(), 4);

c.function("main").begin(); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", ts::i8());

  c.block("entry").begin(); {
    c.assign("arr", literal::array(ts::array(ts::i8(), 4)).push(literal::i8('A')).push(literal::i8('B')).push(literal::i8('C')).push(literal::i8('D')).done());
    c.assign("idx", literal::i8(3));

    auto dest = c.arrayElement("arr", "idx");
    c.callFunction("makeZ", "after_makeZ").into(dest).done();
  } c.endBlock();

  c.block("after_makeZ").begin(); {
    c.writeOut("arr");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("makeZ").ret(ts::i8()).begin(); {
  c.block("entry").begin(); {
    c.returnFromFunction(literal::i8('Z'));
  } c.endBlock();
} c.endFunction();

TEST_END
