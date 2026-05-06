// Tests pass-by-value of an array of i16 values, verifying multi-cell argument copying across a function call.
// Expect: ABCDABCD

TEST_BEGIN
auto array2 = ts::array(ts::i16(), 2);

c.function("main").begin(); {
  c.declareLocal("x", array2);

  auto x0 = c.arrayElement("x", 0);
  auto x1 = c.arrayElement("x", 1);

  c.assign(x0, literal::i16(CAT('A', 'B')));
  c.assign(x1, literal::i16(CAT('C', 'D')));

  c.writeOut("x");
  c.callFunction("foo").arg("x").done();
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("x", array2).ret(ts::void_t()).begin(); {
  c.writeOut("x");
  c.returnFromFunction();
} c.endFunction();

TEST_END
