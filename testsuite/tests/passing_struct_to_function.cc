// Passing entire struct object to function
// Expected: ABABXYAB

TEST_BEGIN

auto point = ts::defineStruct("Point").field("x", ts::i8()).field("y", ts::i8()).done();

c.function("main").begin(); {
  c.declareLocal("s", point);

  auto x = c.structField("s", "x");
  auto y = c.structField("s", "y");

  c.assign(x, literal::i8('A'));
  c.assign(y, literal::i8('B'));
  c.writeOut("s");

  c.callFunction("foo").arg("s").done();
  c.writeOut("s");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("p", point).ret(ts::void_t()).begin(); {
  auto px = c.structField("p", "x");
  auto py = c.structField("p", "y");

  c.writeOut("p");
  c.assign(px, literal::i8('X'));
  c.assign(py, literal::i8('Y'));
  c.writeOut("p");
  c.returnFromFunction();
} c.endFunction();

TEST_END
