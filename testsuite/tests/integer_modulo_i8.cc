// Filename: integer_modulo_i8.cc
// Mod i8 integers using mod and modAssign
// Expected:  !

TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("x", TypeSystem::i8());
  c.declareLocal("y", TypeSystem::i8());
  c.declareLocal("z", TypeSystem::i8());

  c.beginBlock("entry"); {
    c.assign("x", values::i8(100));
    c.assign("y", values::i8(34));

    c.modAssign("x", "y");           // 100 % 34 = 32 -> space
    c.writeOut("x");                 // ' '

    c.assign("x", values::i8(101));
    c.assign("z", c.mod("x", "y"));  // 101 % 34 = 33 -> !
    c.writeOut("z");                 // !

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
