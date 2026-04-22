// Filename: integer_modulo_mixed_i16_i8.cc
// Mod mixed i16/i8 integers using mod and modAssign
// Expected: AABB

TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("x", TypeSystem::i16());
  c.declareLocal("y", TypeSystem::i8());
  c.declareLocal("z", TypeSystem::i16());

  c.beginBlock("entry"); {
    c.assign("x", values::i16(100));
    c.assign("y", values::i8(34));

    c.modAssign("x", "y"); // 100 % 34 = 0x0020
    c.writeOut(c.add("x", values::i16(CAT('!', 'A')))); // 0x0020 + 0x4121 = 0x4141 -> AA

    c.assign("x", values::i16(101));
    c.assign("z", c.add(c.mod("x", "y"), values::i16(CAT('!', 'B')))); // 0x0021 + 0x4221 = 0x4242 -> BB
    c.writeOut("z"); // BB

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
