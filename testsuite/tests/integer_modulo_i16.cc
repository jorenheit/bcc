// Filename: integer_modulo_i16.cc
// Mod i16 integers using mod and modAssign
// Expected:   !!

TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("x", TypeSystem::i16());
  c.declareLocal("y", TypeSystem::i16());
  c.declareLocal("z", TypeSystem::i16());

  c.beginBlock("entry"); {
    c.assign("x", values::i16(0x7070));
    c.assign("y", values::i16(0x5050));

    c.modAssign("x", "y");           // 0x7070 % 0x5050 = 0x2020 -> two spaces
    c.writeOut("x");                 // "  "

    c.assign("x", values::i16(0x7171));
    c.assign("z", c.mod("x", "y"));  // 0x7171 % 0x5050 = 0x2121 -> !!
    c.writeOut("z");                 // !!

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
