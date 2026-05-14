// Signed mixed-width comparisons: s8 with s16 promotes to s16
// Output convention: 'A' = false, 'B' = true.
// Expected: BABAABBABBAB

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("a", ts::s8());
  c.declareLocal("x", ts::s16());

  // s8(-1) < s16(1) -> true
  c.assign("a", literal::s8(-1));
  c.assign("x", literal::s16(1));
  c.write(c.add(c.lt("a", "x"), literal::u8('A'))); // B

  // s16(1) < s8(-1) -> false
  c.write(c.add(c.lt("x", "a"), literal::u8('A'))); // A

  // s8(-128) < s16(-127) -> true
  c.assign("a", literal::s8(-128));
  c.assign("x", literal::s16(-127));
  c.write(c.add(c.lt("a", "x"), literal::u8('A'))); // B

  // s16(-127) < s8(-128) -> false
  c.write(c.add(c.lt("x", "a"), literal::u8('A'))); // A

  // s8(127) < s16(-1) -> false
  c.assign("a", literal::s8(127));
  c.assign("x", literal::s16(-1));
  c.write(c.add(c.lt("a", "x"), literal::u8('A'))); // A

  // s16(-1) < s8(127) -> true
  c.write(c.add(c.lt("x", "a"), literal::u8('A'))); // B

  // s8(-1) > s16(-300) -> true
  c.assign("a", literal::s8(-1));
  c.assign("x", literal::s16(-300));
  c.write(c.add(c.gt("a", "x"), literal::u8('A'))); // B

  // s16(-300) > s8(-1) -> false
  c.write(c.add(c.gt("x", "a"), literal::u8('A'))); // A

  // equality boundaries through promoted comparison
  // s8(-1) <= s16(-1) -> true
  c.assign("a", literal::s8(-1));
  c.assign("x", literal::s16(-1));
  c.write(c.add(c.le("a", "x"), literal::u8('A'))); // B

  // s8(-1) >= s16(-1) -> true
  c.write(c.add(c.ge("a", "x"), literal::u8('A'))); // B

  // s8(-2) >= s16(-1) -> false
  c.assign("a", literal::s8(-2));
  c.assign("x", literal::s16(-1));
  c.write(c.add(c.ge("a", "x"), literal::u8('A'))); // A

  // s16(-1) >= s8(-2) -> true
  c.write(c.add(c.ge("x", "a"), literal::u8('A'))); // B

  c.returnFromFunction();
} c.endFunction();

TEST_END
