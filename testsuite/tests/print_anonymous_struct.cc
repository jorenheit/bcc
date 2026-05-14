// Print an anonymous struct
// Expected: "AB"

TEST_BEGIN
auto point = ts::defineStruct("Point").field("x", ts::u8()).field("y", ts::u8()).done();

c.function("main").begin(); {
  c.declareLocal("s", point);

  c.write(literal::struct_t(point).init("x", literal::u8('A')).init("y", literal::u8('B')).done());
  c.returnFromFunction();
} c.endFunction();

TEST_END
