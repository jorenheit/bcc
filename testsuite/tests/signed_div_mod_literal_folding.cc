// Signed div/mod literal folding
// Expected: ABCDEFGHIJKLMNOP

TEST_BEGIN

c.function("main").begin(); {
  c.block("entry").begin(); {
    // s8 literal folding

    // -7 / 3 = -2; -2 + 'C' = 'A'
    c.writeOut(c.add(c.div(literal::s8(-7), literal::s8(3)),
                     literal::s8('C'))); // A

    // 7 / -3 = -2; -2 + 'D' = 'B'
    c.writeOut(c.add(c.div(literal::s8(7), literal::s8(-3)),
                     literal::s8('D'))); // B

    // -7 / -3 = 2; 2 + 'A' = 'C'
    c.writeOut(c.add(c.div(literal::s8(-7), literal::s8(-3)),
                     literal::s8('A'))); // C

    // 7 / 3 = 2; 2 + 'B' = 'D'
    c.writeOut(c.add(c.div(literal::s8(7), literal::s8(3)),
                     literal::s8('B'))); // D

    // -7 % 3 = -1; -1 + 'F' = 'E'
    c.writeOut(c.add(c.mod(literal::s8(-7), literal::s8(3)),
                     literal::s8('F'))); // E

    // 7 % -3 = 1; 1 + 'E' = 'F'
    c.writeOut(c.add(c.mod(literal::s8(7), literal::s8(-3)),
                     literal::s8('E'))); // F

    // -7 % -3 = -1; -1 + 'H' = 'G'
    c.writeOut(c.add(c.mod(literal::s8(-7), literal::s8(-3)),
                     literal::s8('H'))); // G

    // 7 % 3 = 1; 1 + 'G' = 'H'
    c.writeOut(c.add(c.mod(literal::s8(7), literal::s8(3)),
                     literal::s8('G'))); // H

    // s16 literal folding

    // -7 / 3 = -2 -> IJ
    c.writeOut(c.add(c.div(literal::s16(-7), literal::s16(3)),
                     literal::s16(CAT('K', 'J')))); // IJ

    // 7 / -3 = -2 -> KL
    c.writeOut(c.add(c.div(literal::s16(7), literal::s16(-3)),
                     literal::s16(CAT('M', 'L')))); // KL

    // -7 % 3 = -1 -> MN
    c.writeOut(c.add(c.mod(literal::s16(-7), literal::s16(3)),
                     literal::s16(CAT('N', 'N')))); // MN

    // 7 % -3 = 1 -> OP
    c.writeOut(c.add(c.mod(literal::s16(7), literal::s16(-3)),
                     literal::s16(CAT('N', 'P')))); // OP

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
