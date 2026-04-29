// Self-assignment through a runtime-selected array element should leave the value unchanged.
// Expected: "ABCD"

TEST_BEGIN

auto array4 = ts::array(ts::i8(), 4);

c.function("main").begin(); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", ts::i8());

  c.block("entry").begin(); {
    c.assign("arr", literal::array(ts::array(ts::i8(), 4)).push(literal::i8('A')).push(literal::i8('B')).push(literal::i8('C')).push(literal::i8('D')).done());
    c.assign("idx", literal::i8(2));

    auto elem = c.arrayElement("arr", "idx");
    c.assign(elem, elem);

    c.writeOut("arr");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
