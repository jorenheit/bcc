// Initialize a struct from an anonymous value
// Expected: "AB"

TEST_BEGIN
auto point = ts::defineStruct("Point")("x", ts::i8(),
				     "y", ts::i8());

c.beginFunction("main"); {
  c.declareLocal("s", point);

  c.beginBlock("entry"); {
    c.assign("s", literal::structT(point)(literal::i8('A'),
					 literal::i8('B')));
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
