// Filename: logical_bool.cc
// Test logical bool conversion (lbool / lboolAssign) on i8 args, i16 args,
// literals, variable/literal combinations, and assign variants.
// Each check prints 'A' for false and 'B' for true.
// Expected: ABABABABBA

TEST_BEGIN

auto i8 = ts::i8();
auto i16 = ts::i16();

c.function("main").begin(); {
  c.declareLocal("a", i8);
  c.declareLocal("b", i8);
  c.declareLocal("x", i16);
  c.declareLocal("y", i16);

  // 1. i8 variable: bool(false) -> false
  c.assign("a", literal::i8(0));
  c.write(c.add(c.lbool("a"), literal::i8('A')));

  // 2. i8 variable: bool(true) -> true
  c.assign("a", literal::i8(100));
  c.write(c.add(c.lbool("a"), literal::i8('A')));

  // 3. i16 variable: bool(false) -> false
  c.assign("x", literal::i16(0));
  c.write(c.add(c.lbool("x"), literal::i8('A')));

  // 4. i16 variable: bool(true) -> true
  c.assign("x", literal::i16(0x1234));
  c.write(c.add(c.lbool("x"), literal::i8('A')));

  // 5. i8 literal: bool(false) -> false
  c.write(c.add(c.lbool(literal::i8(0)), literal::i8('A')));

  // 6. i8 literal: bool(true) -> true
  c.write(c.add(c.lbool(literal::i8(77)), literal::i8('A')));

  // 7. i16 literal: bool(false) -> false
  c.write(c.add(c.lbool(literal::i16(0)), literal::i8('A')));

  // 8. i16 literal: bool(true) -> true
  c.write(c.add(c.lbool(literal::i16(0xBEEF)), literal::i8('A')));

  // 9. i8 assign variant: true becomes canonical 1
  c.assign("b", literal::i8(200));
  c.lboolAssign("b");
  c.write(c.add("b", literal::i8('A')));

  // 10. i16 assign variant: false remains canonical 0
  // Verify with eq(...) so the printed value is a single byte boolean.
  c.assign("y", literal::i16(0));
  c.lboolAssign("y");
  c.write(c.add(c.eq("y", literal::i16(1)), literal::i8('A')));

  c.returnFromFunction();
} c.endFunction();

TEST_END
