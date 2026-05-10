// Signed comparisons with slot/const and const/slot operands
// Output convention: 'A' = false, 'B' = true.
// Expected: BABABABABABA

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("a", ts::s8());
  c.declareLocal("x", ts::s16());

  // s8 slot/const: -3 < 2 -> true
  c.assign("a", literal::s8(-3));
  c.writeOut(c.add(c.lt("a", literal::s8(2)), literal::i8('A'))); // B

  // s8 slot/const: -3 > 2 -> false
  c.writeOut(c.add(c.gt("a", literal::s8(2)), literal::i8('A'))); // A

  // s8 const/slot: -10 < -3 -> true
  c.writeOut(c.add(c.lt(literal::s8(-10), "a"), literal::i8('A'))); // B

  // s8 const/slot: 10 < -3 -> false
  c.writeOut(c.add(c.lt(literal::s8(10), "a"), literal::i8('A'))); // A

  // s8 slot/const equality boundary: -128 <= -128 -> true
  c.assign("a", literal::s8(-128));
  c.writeOut(c.add(c.le("a", literal::s8(-128)), literal::i8('A'))); // B

  // s8 slot/const boundary: 127 < -128 -> false
  c.assign("a", literal::s8(127));
  c.writeOut(c.add(c.lt("a", literal::s8(-128)), literal::i8('A'))); // A

  // s16 slot/const: -300 < 200 -> true
  c.assign("x", literal::s16(-300));
  c.writeOut(c.add(c.lt("x", literal::s16(200)), literal::i8('A'))); // B

  // s16 slot/const: -300 > 200 -> false
  c.writeOut(c.add(c.gt("x", literal::s16(200)), literal::i8('A'))); // A

  // s16 const/slot: -1000 < -300 -> true
  c.writeOut(c.add(c.lt(literal::s16(-1000), "x"), literal::i8('A'))); // B

  // s16 const/slot: 1000 < -300 -> false
  c.writeOut(c.add(c.lt(literal::s16(1000), "x"), literal::i8('A'))); // A

  // s16 boundary: -32768 <= -32768 -> true
  c.assign("x", literal::s16(-32768));
  c.writeOut(c.add(c.le("x", literal::s16(-32768)), literal::i8('A'))); // B

  // s16 boundary: 32767 < -32768 -> false
  c.assign("x", literal::s16(32767));
  c.writeOut(c.add(c.lt("x", literal::s16(-32768)), literal::i8('A'))); // A

  c.returnFromFunction();
} c.endFunction();

TEST_END
