// Store the return-value of a function into a struct-field
// Expected: "A"

TEST_BEGIN

auto point = ts::defineStruct("Point").field("x", ts::i8()).field("y", ts::i8()).done();

c.function("main").begin(); {
  c.declareLocal("s", point);

  auto x = c.structField("s", "x");
  auto y = c.structField("s", "y");

  c.assign(x, literal::i8('A'));
  c.callFunction("makeZ").into(y).done();
  c.write("s");
  c.returnFromFunction();
} c.endFunction();

c.function("makeZ").ret(ts::i8()).begin(); {
  c.returnFromFunction(literal::i8('Z'));
} c.endFunction();

TEST_END
