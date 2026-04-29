// Filename: logical_nor.cc
// Test logical NOR (lnor / lnorAssign) on i8 args, i16 args, mixed args,
// variable/literal combinations, two literals for constant folding, and assign variants.
// Each check prints 'A' for false and 'B' for true.
// Expected: AAAAAAAAAB

TEST_BEGIN

auto i8 = ts::i8();
auto i16 = ts::i16();

c.function("main").begin(); {
  c.declareLocal("a", i8);
  c.declareLocal("b", i8);
  c.declareLocal("x", i16);
  c.declareLocal("y", i16);

  c.block("entry").begin(); {
    // 1. i8 variable + i8 variable: true NOR true -> 0
    c.assign("a", literal::i8(0x7B));
    c.assign("b", literal::i8(0x2D));
    c.writeOut(c.add(c.lnor("a", "b"), literal::i8('A')));

    // 2. i8 variable + i8 variable: true NOR false -> 0
    c.assign("a", literal::i8(0x7B));
    c.assign("b", literal::i8(0));
    c.writeOut(c.add(c.lnor("a", "b"), literal::i8('A')));

    // 3. i16 variable + i16 variable: true NOR true -> 0
    c.assign("x", literal::i16(0x1234));
    c.assign("y", literal::i16(0xBEEF));
    c.writeOut(c.add(c.lnor("x", "y"), literal::i8('A')));

    // 4. i16 variable + i16 variable: false NOR true -> 0
    c.assign("x", literal::i16(0));
    c.assign("y", literal::i16(0xCAFE));
    c.writeOut(c.add(c.lnor("x", "y"), literal::i8('A')));

    // 5. mixed i8/i16 variables: true NOR true -> 0
    c.assign("a", literal::i8(0x55));
    c.assign("x", literal::i16(0x4001));
    c.writeOut(c.add(c.lnor("a", "x"), literal::i8('A')));

    // 6. variable + literal: true NOR false -> 0
    c.assign("a", literal::i8(0x66));
    c.writeOut(c.add(c.lnor("a", literal::i8(0)), literal::i8('A')));

    // 7. literal + variable: true NOR true -> 0
    c.assign("x", literal::i16(0x2345));
    c.writeOut(c.add(c.lnor(literal::i16(0x7777), "x"), literal::i8('A')));

    // 8. literal + literal, constant-folded: true NOR false -> 0
    c.writeOut(c.add(c.lnor(literal::i16(0x9999), literal::i16(0)), literal::i8('A')));

    // 9. i8 assign variant: true NOR true -> 0
    c.assign("a", literal::i8(0x42));
    c.lnorAssign("a", literal::i8(0x24));
    c.writeOut(c.add("a", literal::i8('A')));

    // 10. i16 assign variant: false NOR false -> 1
    // Use land(result, 1) to print the boolean value as a single output byte.
    c.assign("x", literal::i16(0));
    c.lnorAssign("x", literal::i16(0));
    c.writeOut(c.add(c.land("x", literal::i16(1)), literal::i8('A')));

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
