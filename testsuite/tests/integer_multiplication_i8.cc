// Filename: integer_multiplication_i8.cc
// Multiply i8 integers using mul and mulAssign
// Expected: AM

TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("x", TypeSystem::i8());
  c.declareLocal("y", TypeSystem::i8());
  c.declareLocal("z", TypeSystem::i8());

  c.beginBlock("entry"); {
    c.assign("x", values::i8(5));
    c.assign("y", values::i8(13));

    c.mulAssign("x", "y");           // 5 * 13 = 65 -> A
    c.writeOut("x");                 // A
    c.assign("z", c.mul("x", "y"));  // 65 * 13 = 845 mod 256 = 77 -> M
    c.writeOut("z");                 // M

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
