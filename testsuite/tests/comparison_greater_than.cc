// Greater-than comparison tests (gt / gtAssign)
// Covers: i8/i8, i16/i16, mixed i8/i16, variable/literal,
// literal/variable, literal/literal constant folding, and assign variants.
// Output convention: 'A' = false, 'B' = true.
// Expected output: ABABABABAAABA

TEST_BEGIN

auto i8 = ts::i8();
auto i16 = ts::i16();

c.function("main").begin(); {
  c.declareLocal("a", i8);
  c.declareLocal("b", i8);
  c.declareLocal("x", i16);
  c.declareLocal("y", i16);

  // i8/i8 variables: 20 ? 30, then 30 ? 20
  c.assign("a", literal::i8(20));
  c.assign("b", literal::i8(30));
  c.write(c.add(c.gt("a", "b"), literal::i8('A'))); // A
  c.write(c.add(c.gt("b", "a"), literal::i8('A'))); // B

  // i16/i16 variables with values larger than one byte:
  // 2000 ? 3000, then 3000 ? 2000
  c.assign("x", literal::i16(2000));
  c.assign("y", literal::i16(3000));
  c.write(c.add(c.gt("x", "y"), literal::i8('A'))); // A
  c.write(c.add(c.gt("y", "x"), literal::i8('A'))); // B

  // mixed i8/i16 variables: 20 ? 2000, then 2000 ? 20
  c.write(c.add(c.gt("a", "x"), literal::i8('A'))); // A
  c.write(c.add(c.gt("x", "a"), literal::i8('A'))); // B

  // variable/literal equality edge: 20 ? 20
  c.write(c.add(c.gt("a", literal::i8(20)), literal::i8('A'))); // A

  // literal/variable with larger i16 literal: 4096 ? 3000
  c.write(c.add(c.gt(literal::i16(4096), "y"), literal::i8('A'))); // B

  // literal/literal constant folding, mixed widths: 42 ? 42
  c.write(c.add(c.gt(literal::i8(42), literal::i16(42)), literal::i8('A'))); // A

  // assign variant, i8/i8 variables: 7 ? 9
  c.assign("a", literal::i8(7));
  c.assign("b", literal::i8(9));
  c.gtAssign("a", "b");
  c.write(c.add("a", literal::i8('A')));  // A

  // assign variant, i16 variable/literal: 5000 ? 4000
  c.assign("x", literal::i16(5000));
  c.gtAssign("x", literal::i16(4000));
  c.write(c.add("x", literal::i16(CAT('A', 'B')))); // BB

  // assign variant, mixed i8/i16 variables: 250 ? 1000
  c.assign("a", literal::i8(250));
  c.assign("x", literal::i16(1000));
  c.gtAssign("a", "x");
  c.write(c.add("a", literal::i8('A'))); // A

  c.returnFromFunction();
} c.endFunction();

TEST_END
