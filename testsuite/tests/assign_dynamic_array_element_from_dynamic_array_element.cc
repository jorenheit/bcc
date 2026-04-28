// Copy from one runtime-selected array element to another runtime-selected array element.
// Expected: "ABAD"

TEST_BEGIN

auto array4 = ts::array(ts::i8(), 4);

c.beginFunction("main"); {
  c.declareLocal("arr", array4);
  c.declareLocal("srcIdx", ts::i8());
  c.declareLocal("dstIdx", ts::i8());

  c.beginBlock("entry"); {
    c.assign("arr", literal::array(ts::i8())(literal::i8('A'),
						    literal::i8('B'),
						    literal::i8('C'),
						    literal::i8('D')));
    c.assign("srcIdx", literal::i8(0));
    c.assign("dstIdx", literal::i8(2));

    auto src = c.arrayElement("arr", "srcIdx");
    auto dst = c.arrayElement("arr", "dstIdx");
    c.assign(dst, src);

    c.writeOut("arr");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
