// Filename: integer_modulo_mixed_i16_i8.cc
// Mod mixed i16/i8 integers using mod and modAssign
// Expected: AABB

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i16());
  c.declareLocal("y", ts::i8());
  c.declareLocal("z", ts::i16());

  c.block("entry").begin(); {
    c.assign("x", literal::i16(100));
    c.assign("y", literal::i8(34));

    c.modAssign("x", "y"); // 100 % 34 = 0x0020
    c.writeOut(c.add("x", literal::i16(CAT('!', 'A')))); // 0x0020 + 0x4121 = 0x4141 -> AA

    c.assign("x", literal::i16(101));
    c.assign("z", c.add(c.mod("x", "y"), literal::i16(CAT('!', 'B')))); // 0x0021 + 0x4221 = 0x4242 -> BB
    c.writeOut("z"); // BB

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
