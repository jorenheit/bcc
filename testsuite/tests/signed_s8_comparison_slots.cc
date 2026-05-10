// Signed s8 slot/slot comparisons
// Output convention: 'A' = false, 'B' = true.
// Expected: BABAABABBBAB

TEST_BEGIN


c.function("main").begin(); {
  c.declareLocal("x", ts::s8());
  c.declareLocal("y", ts::s8());

  // -1 < 0 -> true
  c.assign("x", literal::s8(-1));
  c.assign("y", literal::s8(0));
  c.writeOut(c.add(c.lt("x", "y"), literal::i8('A'))); // B

  // 0 < -1 -> false
  c.writeOut(c.add(c.lt("y", "x"), literal::i8('A'))); // A

  // -5 < -2 -> true
  c.assign("x", literal::s8(-5));
  c.assign("y", literal::s8(-2));
  c.writeOut(c.add(c.lt("x", "y"), literal::i8('A'))); // B

  // -2 < -5 -> false
  c.writeOut(c.add(c.lt("y", "x"), literal::i8('A'))); // A

  // 127 < -128 -> false
  c.assign("x", literal::s8(127));
  c.assign("y", literal::s8(-128));
  c.writeOut(c.add(c.lt("x", "y"), literal::i8('A'))); // A

  // -128 < 127 -> true
  c.writeOut(c.add(c.lt("y", "x"), literal::i8('A'))); // B

  // -1 > 0 -> false
  c.assign("x", literal::s8(-1));
  c.assign("y", literal::s8(0));
  c.writeOut(c.add(c.gt("x", "y"), literal::i8('A'))); // A

  // 0 > -1 -> true
  c.writeOut(c.add(c.gt("y", "x"), literal::i8('A'))); // B

  // -5 <= -5 -> true
  c.assign("x", literal::s8(-5));
  c.assign("y", literal::s8(-5));
  c.writeOut(c.add(c.le("x", "y"), literal::i8('A'))); // B

  // -5 >= -5 -> true
  c.writeOut(c.add(c.ge("x", "y"), literal::i8('A'))); // B

  // -5 >= -2 -> false
  c.assign("x", literal::s8(-5));
  c.assign("y", literal::s8(-2));
  c.writeOut(c.add(c.ge("x", "y"), literal::i8('A'))); // A

  // -2 >= -5 -> true
  c.writeOut(c.add(c.ge("y", "x"), literal::i8('A'))); // B

  c.returnFromFunction();
} c.endFunction();

TEST_END
