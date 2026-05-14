// Filename: logical_nand.cc
// Test logical NAND (lnand / lnandAssign) on u8 args, u16 args, mixed args,
// variable/literal combinations, two literals for constant folding, and assign variants.
// Each check prints 'A' for false and 'B' for true.
// Expected: ABABABABAB

TEST_BEGIN

auto u8 = ts::u8();
auto u16 = ts::u16();

c.function("main").begin(); {
  c.declareLocal("a", u8);
  c.declareLocal("b", u8);
  c.declareLocal("x", u16);
  c.declareLocal("y", u16);

  // 1. u8 variable + u8 variable: true NAND true -> 0
  c.assign("a", literal::u8(0x7B));
  c.assign("b", literal::u8(0x2D));
  c.write(c.add(c.lnand("a", "b"), literal::u8('A')));

  // 2. u8 variable + u8 variable: true NAND false -> 1
  c.assign("a", literal::u8(0x7B));
  c.assign("b", literal::u8(0));
  c.write(c.add(c.lnand("a", "b"), literal::u8('A')));

  // 3. u16 variable + u16 variable: true NAND true -> 0
  c.assign("x", literal::u16(0x1234));
  c.assign("y", literal::u16(0xBEEF));
  c.write(c.add(c.lnand("x", "y"), literal::u8('A')));

  // 4. u16 variable + u16 variable: false NAND true -> 1
  c.assign("x", literal::u16(0));
  c.assign("y", literal::u16(0xCAFE));
  c.write(c.add(c.lnand("x", "y"), literal::u8('A')));

  // 5. mixed u8/u16 variables: true NAND true -> 0
  c.assign("a", literal::u8(0x55));
  c.assign("x", literal::u16(0x4001));
  c.write(c.add(c.lnand("a", "x"), literal::u8('A')));

  // 6. variable + literal: true NAND false -> 1
  c.assign("a", literal::u8(0x66));
  c.write(c.add(c.lnand("a", literal::u8(0)), literal::u8('A')));

  // 7. literal + variable: true NAND true -> 0
  c.assign("x", literal::u16(0x2345));
  c.write(c.add(c.lnand(literal::u16(0x7777), "x"), literal::u8('A')));

  // 8. literal + literal, constant-folded: true NAND false -> 1
  c.write(c.add(c.lnand(literal::u16(0x9999), literal::u16(0)), literal::u8('A')));

  // 9. u8 assign variant: true NAND true -> 0
  c.assign("a", literal::u8(0x42));
  c.lnandAssign("a", literal::u8(0x24));
  c.write(c.add("a", literal::u8('A')));

  // 10. u16 assign variant: false NAND false -> 1
  // Use land(result, 1) to print the boolean value as a single output byte.
  c.assign("x", literal::u16(0));
  c.lnandAssign("x", literal::u16(0));
  c.write(c.add(c.land("x", literal::u16(1)), literal::u8('A')));

  c.returnFromFunction();
} c.endFunction();

TEST_END
