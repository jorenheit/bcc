// Read from an array element selected by a runtime index into a normal local.
// Expected: "C"

TEST_BEGIN

auto array4 = ts::array(ts::i8(), 4);

c.beginFunction("main"); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", ts::i8());
  c.declareLocal("out", ts::i8());

  c.beginBlock("entry"); {
    c.assign("arr", literal::array(ts::i8())(literal::i8('A'),
						    literal::i8('B'),
						    literal::i8('C'),
						    literal::i8('D')));
    c.assign("idx", literal::i8(2));

    auto elem = c.arrayElement("arr", "idx");
    c.assign("out", elem);

    c.writeOut("out");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
