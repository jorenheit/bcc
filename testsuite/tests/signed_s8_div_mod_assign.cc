// Signed s8 divAssign and modAssign
// Expected: ABCDEFGH

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::s8());
  c.declareLocal("y", ts::s8());

  // -7 / 3 = -2; -2 + 'C' = 'A'
  c.assign("x", literal::s8(-7));
  c.assign("y", literal::s8(3));
  c.divAssign("x", "y");
  c.writeOut(c.add("x", literal::s8('C'))); // A

  // 7 / -3 = -2; -2 + 'D' = 'B'
  c.assign("x", literal::s8(7));
  c.assign("y", literal::s8(-3));
  c.divAssign("x", "y");
  c.writeOut(c.add("x", literal::s8('D'))); // B

  // -7 / -3 = 2; 2 + 'A' = 'C'
  c.assign("x", literal::s8(-7));
  c.assign("y", literal::s8(-3));
  c.divAssign("x", "y");
  c.writeOut(c.add("x", literal::s8('A'))); // C

  // 7 / 3 = 2; 2 + 'B' = 'D'
  c.assign("x", literal::s8(7));
  c.assign("y", literal::s8(3));
  c.divAssign("x", "y");
  c.writeOut(c.add("x", literal::s8('B'))); // D

  // -7 % 3 = -1; -1 + 'F' = 'E'
  c.assign("x", literal::s8(-7));
  c.assign("y", literal::s8(3));
  c.modAssign("x", "y");
  c.writeOut(c.add("x", literal::s8('F'))); // E

  // 7 % -3 = 1; 1 + 'E' = 'F'
  c.assign("x", literal::s8(7));
  c.assign("y", literal::s8(-3));
  c.modAssign("x", "y");
  c.writeOut(c.add("x", literal::s8('E'))); // F

  // -7 % -3 = -1; -1 + 'H' = 'G'
  c.assign("x", literal::s8(-7));
  c.assign("y", literal::s8(-3));
  c.modAssign("x", "y");
  c.writeOut(c.add("x", literal::s8('H'))); // G

  // 7 % 3 = 1; 1 + 'G' = 'H'
  c.assign("x", literal::s8(7));
  c.assign("y", literal::s8(3));
  c.modAssign("x", "y");
  c.writeOut(c.add("x", literal::s8('G'))); // H

  c.returnFromFunction();
} c.endFunction();

TEST_END
