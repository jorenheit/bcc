// Filename: integer_modulo_i16.cc
// Mod i16 integers using mod and modAssign
// Expected:   !!

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i16());
  c.declareLocal("y", ts::i16());
  c.declareLocal("z", ts::i16());

  c.assign("x", literal::i16(0x7070));
  c.assign("y", literal::i16(0x5050));

  c.modAssign("x", "y");           // 0x7070 % 0x5050 = 0x2020 -> two spaces
  c.write("x");                 // "  "

  c.assign("x", literal::i16(0x7171));
  c.assign("z", c.mod("x", "y"));  // 0x7171 % 0x5050 = 0x2121 -> !!
  c.write("z");                 // !!

  c.returnFromFunction();
} c.endFunction();

TEST_END
