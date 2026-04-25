// Greater-than comparison tests (gt / gtAssign)
// Covers: i8/i8, i16/i16, mixed i8/i16, variable/literal,
// literal/variable, literal/literal constant folding, and assign variants.
// Output convention: 'A' = false, 'B' = true.
// Expected output: ABABABABAAABA

TEST_BEGIN

    auto i8 = TypeSystem::i8();
    auto i16 = TypeSystem::i16();

    c.beginFunction("main"); {
      c.declareLocal("a", i8);
      c.declareLocal("b", i8);
      c.declareLocal("x", i16);
      c.declareLocal("y", i16);

      c.beginBlock("entry"); {

        // i8/i8 variables: 20 ? 30, then 30 ? 20
        c.assign("a", values::i8(20));
        c.assign("b", values::i8(30));
        c.writeOut(c.add(c.gt("a", "b"), values::i8('A'))); // A
        c.writeOut(c.add(c.gt("b", "a"), values::i8('A'))); // B

        // i16/i16 variables with values larger than one byte:
        // 2000 ? 3000, then 3000 ? 2000
        c.assign("x", values::i16(2000));
        c.assign("y", values::i16(3000));
        c.writeOut(c.add(c.gt("x", "y"), values::i8('A'))); // A
        c.writeOut(c.add(c.gt("y", "x"), values::i8('A'))); // B

        // mixed i8/i16 variables: 20 ? 2000, then 2000 ? 20
        c.writeOut(c.add(c.gt("a", "x"), values::i8('A'))); // A
        c.writeOut(c.add(c.gt("x", "a"), values::i8('A'))); // B

        // variable/literal equality edge: 20 ? 20
        c.writeOut(c.add(c.gt("a", values::i8(20)), values::i8('A'))); // A

        // literal/variable with larger i16 literal: 4096 ? 3000
        c.writeOut(c.add(c.gt(values::i16(4096), "y"), values::i8('A'))); // B

        // literal/literal constant folding, mixed widths: 42 ? 42
        c.writeOut(c.add(c.gt(values::i8(42), values::i16(42)), values::i8('A'))); // A

        // assign variant, i8/i8 variables: 7 ? 9
        c.assign("a", values::i8(7));
        c.assign("b", values::i8(9));
        c.gtAssign("a", "b");
        c.writeOut(c.add("a", values::i8('A')));  // A

        // assign variant, i16 variable/literal: 5000 ? 4000
        c.assign("x", values::i16(5000));
        c.gtAssign("x", values::i16(4000));
        c.writeOut(c.add("x", values::i16(CAT('A', 'B')))); // BB

        // assign variant, mixed i8/i16 variables: 250 ? 1000
        c.assign("a", values::i8(250));
        c.assign("x", values::i16(1000));
        c.gtAssign("a", "x");
        c.writeOut(c.add("a", values::i8('A'))); // A

        c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

TEST_END
