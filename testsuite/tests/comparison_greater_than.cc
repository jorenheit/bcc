// Greater-than comparison tests (gt / gtAssign)
// Covers: u8/u8, u16/u16, mixed u8/u16, variable/literal,
// literal/variable, literal/literal constant folding, and assign variants.
// Output convention: 'A' = false, 'B' = true.
// Expected output: ABABABABAAABA

TEST_BEGIN

auto u8 = ts::u8();
auto u16 = ts::u16();

c.function("main").begin(); {
  c.declareLocal("a", u8);
  c.declareLocal("b", u8);
  c.declareLocal("x", u16);
  c.declareLocal("y", u16);

  // u8/u8 variables: 20 ? 30, then 30 ? 20
  c.assign("a", literal::u8(20));
  c.assign("b", literal::u8(30));
  c.write(c.add(c.gt("a", "b"), literal::u8('A'))); // A
  c.write(c.add(c.gt("b", "a"), literal::u8('A'))); // B

  // u16/u16 variables with values larger than one byte:
  // 2000 ? 3000, then 3000 ? 2000
  c.assign("x", literal::u16(2000));
  c.assign("y", literal::u16(3000));
  c.write(c.add(c.gt("x", "y"), literal::u8('A'))); // A
  c.write(c.add(c.gt("y", "x"), literal::u8('A'))); // B

  // mixed u8/u16 variables: 20 ? 2000, then 2000 ? 20
  c.write(c.add(c.gt("a", "x"), literal::u8('A'))); // A
  c.write(c.add(c.gt("x", "a"), literal::u8('A'))); // B

  // variable/literal equality edge: 20 ? 20
  c.write(c.add(c.gt("a", literal::u8(20)), literal::u8('A'))); // A

  // literal/variable with larger u16 literal: 4096 ? 3000
  c.write(c.add(c.gt(literal::u16(4096), "y"), literal::u8('A'))); // B

  // literal/literal constant folding, mixed widths: 42 ? 42
  c.write(c.add(c.gt(literal::u8(42), literal::u16(42)), literal::u8('A'))); // A

  // assign variant, u8/u8 variables: 7 ? 9
  c.assign("a", literal::u8(7));
  c.assign("b", literal::u8(9));
  c.gtAssign("a", "b");
  c.write(c.add("a", literal::u8('A')));  // A

  // assign variant, u16 variable/literal: 5000 ? 4000
  c.assign("x", literal::u16(5000));
  c.gtAssign("x", literal::u16(4000));
  c.write(c.add("x", literal::u16(CAT('A', 'B')))); // BB

  // assign variant, mixed u8/u16 variables: 250 ? 1000
  c.assign("a", literal::u8(250));
  c.assign("x", literal::u16(1000));
  c.gtAssign("a", "x");
  c.write(c.add("a", literal::u8('A'))); // A

  c.returnFromFunction();
} c.endFunction();

TEST_END
