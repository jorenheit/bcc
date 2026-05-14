// Filename: logical_bool.cc
// Test logical bool conversion (lbool / lboolAssign) on u8 args, u16 args,
// literals, variable/literal combinations, and assign variants.
// Each check prints 'A' for false and 'B' for true.
// Expected: ABABABABBA

TEST_BEGIN

auto u8 = ts::u8();
auto u16 = ts::u16();

c.function("main").begin(); {
  c.declareLocal("a", u8);
  c.declareLocal("b", u8);
  c.declareLocal("x", u16);
  c.declareLocal("y", u16);

  // 1. u8 variable: bool(false) -> false
  c.assign("a", literal::u8(0));
  c.write(c.add(c.lbool("a"), literal::u8('A')));

  // 2. u8 variable: bool(true) -> true
  c.assign("a", literal::u8(100));
  c.write(c.add(c.lbool("a"), literal::u8('A')));

  // 3. u16 variable: bool(false) -> false
  c.assign("x", literal::u16(0));
  c.write(c.add(c.lbool("x"), literal::u8('A')));

  // 4. u16 variable: bool(true) -> true
  c.assign("x", literal::u16(0x1234));
  c.write(c.add(c.lbool("x"), literal::u8('A')));

  // 5. u8 literal: bool(false) -> false
  c.write(c.add(c.lbool(literal::u8(0)), literal::u8('A')));

  // 6. u8 literal: bool(true) -> true
  c.write(c.add(c.lbool(literal::u8(77)), literal::u8('A')));

  // 7. u16 literal: bool(false) -> false
  c.write(c.add(c.lbool(literal::u16(0)), literal::u8('A')));

  // 8. u16 literal: bool(true) -> true
  c.write(c.add(c.lbool(literal::u16(0xBEEF)), literal::u8('A')));

  // 9. u8 assign variant: true becomes canonical 1
  c.assign("b", literal::u8(200));
  c.lboolAssign("b");
  c.write(c.add("b", literal::u8('A')));

  // 10. u16 assign variant: false remains canonical 0
  // Verify with eq(...) so the printed value is a single byte boolean.
  c.assign("y", literal::u16(0));
  c.lboolAssign("y");
  c.write(c.add(c.eq("y", literal::u16(1)), literal::u8('A')));

  c.returnFromFunction();
} c.endFunction();

TEST_END
