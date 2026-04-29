// Write to a runtime-selected array element, make a function call, then read the same element again.
// Expected: "X"

TEST_BEGIN

auto array4 = ts::array(ts::i8(), 4);

c.function("main").begin(); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", ts::i8());
  c.declareLocal("out", ts::i8());

  c.block("entry").begin(); {
    c.assign("arr", literal::array(ts::array(ts::i8(), 4)).push(literal::i8('A')).push(literal::i8('B')).push(literal::i8('C')).push(literal::i8('D')).done());
    c.assign("idx", literal::i8(1));

    auto elem = c.arrayElement("arr", "idx");
    c.assign(elem, literal::i8('X'));

    c.callFunction("noop", "after_call").done();
  } c.endBlock();

  c.block("after_call").begin(); {
    auto elem = c.arrayElement("arr", "idx");
    c.assign("out", elem);
    c.writeOut("out");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("noop").begin(); {
  c.block("entry").begin(); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
