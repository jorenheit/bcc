// Filename: logical_xor.cc
// Test logical XOR (lxor / lxorAssign) on i8 args, i16 args, mixed args,
// variable/literal combinations, two literals for constant folding, and assign variants.
// Each check prints 'A' for false and 'B' for true.
// Expected: ABABABABAA

TEST_BEGIN

auto i8 = TypeSystem::i8();
auto i16 = TypeSystem::i16();

c.beginFunction("main"); {
  c.declareLocal("a", i8);
  c.declareLocal("b", i8);
  c.declareLocal("x", i16);
  c.declareLocal("y", i16);

  c.beginBlock("entry"); {
    // 1. i8 variable + i8 variable: true XOR true -> 0
    c.assign("a", values::i8(0x7B));
    c.assign("b", values::i8(0x2D));
    c.writeOut(c.add(c.lxor("a", "b"), values::i8('A')));

    // 2. i8 variable + i8 variable: true XOR false -> 1
    c.assign("a", values::i8(0x7B));
    c.assign("b", values::i8(0));
    c.writeOut(c.add(c.lxor("a", "b"), values::i8('A')));

    // 3. i16 variable + i16 variable: true XOR true -> 0
    c.assign("x", values::i16(0x1234));
    c.assign("y", values::i16(0xBEEF));
    c.writeOut(c.add(c.lxor("x", "y"), values::i8('A')));

    // 4. i16 variable + i16 variable: false XOR true -> 1
    c.assign("x", values::i16(0));
    c.assign("y", values::i16(0xCAFE));
    c.writeOut(c.add(c.lxor("x", "y"), values::i8('A')));

    // 5. mixed i8/i16 variables: true XOR true -> 0
    c.assign("a", values::i8(0x55));
    c.assign("x", values::i16(0x4001));
    c.writeOut(c.add(c.lxor("a", "x"), values::i8('A')));

    // 6. variable + literal: true XOR false -> 1
    c.assign("a", values::i8(0x66));
    c.writeOut(c.add(c.lxor("a", values::i8(0)), values::i8('A')));

    // 7. literal + variable: true XOR true -> 0
    c.assign("x", values::i16(0x2345));
    c.writeOut(c.add(c.lxor(values::i16(0x7777), "x"), values::i8('A')));

    // 8. literal + literal, constant-folded: true XOR false -> 1
    c.writeOut(c.add(c.lxor(values::i16(0x9999), values::i16(0)), values::i8('A')));

    // 9. i8 assign variant: true XOR true -> 0
    c.assign("a", values::i8(0x42));
    c.lxorAssign("a", values::i8(0x24));
    c.writeOut(c.add("a", values::i8('A')));

    // 10. i16 assign variant: false XOR false -> 0
    // Use land(result, 1) to print the boolean value as a single output byte.
    c.assign("x", values::i16(0));
    c.lxorAssign("x", values::i16(0));
    c.writeOut(c.add(c.land("x", values::i16(1)), values::i8('A')));

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
