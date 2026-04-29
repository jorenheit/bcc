// Print an anonymous struct
// Expected: "AB"

TEST_BEGIN
auto point = ts::defineStruct("Point").field("x", ts::i8()).field("y", ts::i8()).done();

c.function("main").begin(); {
  c.declareLocal("s", point);

  c.block("entry").begin(); {
    c.writeOut(literal::struct_t(point).init("x", literal::i8('A')).init("y", literal::i8('B')).done());
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
