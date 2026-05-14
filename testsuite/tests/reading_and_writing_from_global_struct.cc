// Reading from, writing to (mutating) fields of a global struct variable
// Expected: ABAC


TEST_BEGIN

auto point = ts::defineStruct("Point").field("x", ts::u8()).field("y", ts::u8()).done();

c.declareGlobal("g", point);

c.function("main").begin(); {
  c.referGlobals({"g"});

  auto gx = c.structField("g", "x");
  auto gy = c.structField("g", "y");

  c.assign(gx, literal::u8('A'));
  c.assign(gy, literal::u8('B'));
  c.write("g");

  c.callFunction("foo").done();
  c.write("g");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").begin(); {
  c.referGlobals({"g"});

  auto gy = c.structField("g", "y");
  c.assign(gy, literal::u8('C'));
  c.returnFromFunction();
} c.endFunction();

TEST_END
