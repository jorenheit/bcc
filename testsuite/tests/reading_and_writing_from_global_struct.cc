// Reading from, writing to (mutating) fields of a global struct variable
// Expected: ABAC


TEST_BEGIN

auto point = ts::defineStruct("Point").field("x", ts::i8()).field("y", ts::i8()).done();

c.declareGlobal("g", point);

c.function("main").begin(); {
  c.referGlobals({"g"});

  auto gx = c.structField("g", "x");
  auto gy = c.structField("g", "y");

  c.assign(gx, literal::i8('A'));
  c.assign(gy, literal::i8('B'));
  c.writeOut("g");

  c.callFunction("foo").done();
  c.writeOut("g");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").begin(); {
  c.referGlobals({"g"});

  auto gy = c.structField("g", "y");
  c.assign(gy, literal::i8('C'));
  c.returnFromFunction();
} c.endFunction();

TEST_END
