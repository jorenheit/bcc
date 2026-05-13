// Initialize a struct from an anonymous value
// Expected: "AB"

TEST_BEGIN
auto point = ts::defineStruct("Point").field("x", ts::i8()).field("y", ts::i8()).done();

c.function("main").begin(); {
  c.declareLocal("s", point);

  c.assign("s", literal::struct_t(point).init("x", literal::i8('A')).init("y", literal::i8('B')).done());
  c.write("s");
  c.returnFromFunction();
} c.endFunction();

TEST_END
