// Print an anonymous struct
// Expected: "AB"

TEST_BEGIN
auto point = ts::defineStruct("Point")("x", ts::i8(),
				     "y", ts::i8());

c.beginFunction("main"); {
  c.declareLocal("s", point);

  c.beginBlock("entry"); {
    c.writeOut(literal::structT(point)(literal::i8('A'),
				      literal::i8('B')));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
