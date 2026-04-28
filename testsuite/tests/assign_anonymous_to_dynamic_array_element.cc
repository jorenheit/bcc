// Assign an anonymous i8 value into an array element selected by a runtime index.
// Expected: "ABZD"

TEST_BEGIN

auto array4 = ts::array(ts::i8(), 4);

c.beginFunction("main"); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", ts::i8());

  c.beginBlock("entry"); {
    c.assign("arr", literal::array(ts::i8())(literal::i8('A'),
						    literal::i8('B'),
						    literal::i8('C'),
						    literal::i8('D')));
    
    c.assign("idx", literal::i8(2));

    auto elem = c.arrayElement("arr", "idx");
    c.assign(elem, literal::i8('Z'));

    c.writeOut("arr");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
