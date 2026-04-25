// Filename: integer_multiplication_i16.cc
// Multiply i16 integers using mul and mulAssign
// Expected: ""DD

TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("x", TypeSystem::i16());
  c.declareLocal("y", TypeSystem::i16());
  c.declareLocal("z", TypeSystem::i16());

  c.beginBlock("entry"); {
    c.assign("x", values::i16(0x1111));
    c.assign("y", values::i16(2));

    c.mulAssign("x", "y");           // 0x1111 * 2 = 0x2222 -> ""
    c.writeOut("x");                 // ""
    c.assign("z", c.mul("x", "y"));  // 0x2222 * 2 = 0x4444 -> DD
    c.writeOut("z");                 // DD

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
