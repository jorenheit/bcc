// Tests writing to struct fields and reading back from them.
// Expected: "ABAB"


TEST_BEGIN

auto point = ts::defineStruct("Point").field("x", ts::u8()).field("y", ts::u8()).done();

c.function("main").begin(); {
  c.declareLocal("s", point);


  auto x = c.structField("s", "x");
  auto y = c.structField("s", "y");

  c.assign(x, literal::u8('A'));
  c.assign(y, literal::u8('B'));

  c.write(x);
  c.write(y);
  c.write("s");

  c.returnFromFunction();
} c.endFunction();


TEST_END
