// Return an entire struct object from a function
// Expected: "QR"

TEST_BEGIN
auto point = ts::defineStruct("Point").field("x", ts::i8()).field("y", ts::i8()).done();

c.function("main").begin(); {
  c.declareLocal("s", point);

  c.callFunction("makePoint").into("s").done();
  c.write("s");
  c.returnFromFunction();
} c.endFunction();

c.function("makePoint").ret(point).begin(); {
  c.declareLocal("p", point);
  auto x = c.structField("p", "x");
  auto y = c.structField("p", "y");

  c.assign(x, literal::i8('Q'));
  c.assign(y, literal::i8('R'));
  c.returnFromFunction("p");
} c.endFunction();

TEST_END
