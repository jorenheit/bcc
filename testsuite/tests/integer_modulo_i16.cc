// Filename: integer_modulo_u16.cc
// Mod u16 integers using mod and modAssign
// Expected:   !!

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u16());
  c.declareLocal("y", ts::u16());
  c.declareLocal("z", ts::u16());

  c.assign("x", literal::u16(0x7070));
  c.assign("y", literal::u16(0x5050));

  c.modAssign("x", "y");           // 0x7070 % 0x5050 = 0x2020 -> two spaces
  c.write("x");                 // "  "

  c.assign("x", literal::u16(0x7171));
  c.assign("z", c.mod("x", "y"));  // 0x7171 % 0x5050 = 0x2121 -> !!
  c.write("z");                 // !!

  c.returnFromFunction();
} c.endFunction();

TEST_END
