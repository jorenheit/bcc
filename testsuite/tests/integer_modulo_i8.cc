// Filename: integer_modulo_i8.cc
// Mod i8 integers using mod and modAssign
// Expected:  !

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i8());
  c.declareLocal("y", ts::i8());
  c.declareLocal("z", ts::i8());

  c.assign("x", literal::i8(100));
  c.assign("y", literal::i8(34));

  c.modAssign("x", "y");           // 100 % 34 = 32 -> space
  c.writeOut("x");                 // ' '

  c.assign("x", literal::i8(101));
  c.assign("z", c.mod("x", "y"));  // 101 % 34 = 33 -> !
  c.writeOut("z");                 // !

  c.returnFromFunction();
} c.endFunction();

TEST_END
