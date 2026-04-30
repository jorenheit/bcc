// Filename: logical_not.cc
// Test logical NOT (lnot / lnotAssign) on i8 args, i16 args,
// literals, variable/literal combinations, and assign variants.
// Each check prints 'A' for false and 'B' for true.
// Expected: BABABABABA

TEST_BEGIN

auto i8 = ts::i8();
auto i16 = ts::i16();

c.function("main").begin(); {
  c.declareLocal("a", i8);
  c.declareLocal("b", i8);
  c.declareLocal("x", i16);
  c.declareLocal("y", i16);

  c.block("entry").begin(); {
    // 1. i8 variable: NOT false -> true
    c.assign("a", literal::i8(0));
    c.writeOut(c.add(c.lnot("a"), literal::i8('A')));

    // 2. i8 variable: NOT true -> false
    c.assign("a", literal::i8(100));
    c.writeOut(c.add(c.lnot("a"), literal::i8('A')));

    // 3. i16 variable: NOT false -> true
    c.assign("x", literal::i16(0));
    c.writeOut(c.add(c.lnot("x"), literal::i8('A')));

    // 4. i16 variable: NOT true -> false
    c.assign("x", literal::i16(0x1234));
    c.writeOut(c.add(c.lnot("x"), literal::i8('A')));

    // 5. i8 literal: NOT false -> true
    c.writeOut(c.add(c.lnot(literal::i8(0)), literal::i8('A')));

    // 6. i8 literal: NOT true -> false
    c.writeOut(c.add(c.lnot(literal::i8(77)), literal::i8('A')));

    // 7. i16 literal: NOT false -> true
    c.writeOut(c.add(c.lnot(literal::i16(0)), literal::i8('A')));

    // 8. i16 literal: NOT true -> false
    c.writeOut(c.add(c.lnot(literal::i16(0xBEEF)), literal::i8('A')));

    // 9. i8 assign variant: false becomes true
    c.assign("b", literal::i8(0));
    c.lnotAssign("b");
    c.writeOut(c.add("b", literal::i8('A')));

    // 10. i16 assign variant: true becomes false
    // Verify with eq(...) so the printed value is a single byte boolean.
    c.assign("y", literal::i16(0xCAFE));
    c.lnotAssign("y");
    c.writeOut(c.add(c.eq("y", literal::i16(0)), literal::i8('A')));

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
