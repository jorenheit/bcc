// Signed s8 division and modulo with slot/const and const/slot operands
// Expected: ABCDEFGH

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::s8());
  c.declareLocal("z", ts::s8());

  // slot / const: -12 / 5 = -2; -2 + 'C' = 'A'
  c.assign("x", literal::s8(-12));
  c.assign("z", c.div("x", literal::s8(5)));
  c.writeOut(c.add("z", literal::s8('C'))); // A

  // slot / const: 12 / -5 = -2; -2 + 'D' = 'B'
  c.assign("x", literal::s8(12));
  c.assign("z", c.div("x", literal::s8(-5)));
  c.writeOut(c.add("z", literal::s8('D'))); // B

  // const / slot: -12 / 5 = -2; -2 + 'E' = 'C'
  c.assign("x", literal::s8(5));
  c.assign("z", c.div(literal::s8(-12), "x"));
  c.writeOut(c.add("z", literal::s8('E'))); // C

  // const / slot: 12 / -5 = -2; -2 + 'F' = 'D'
  c.assign("x", literal::s8(-5));
  c.assign("z", c.div(literal::s8(12), "x"));
  c.writeOut(c.add("z", literal::s8('F'))); // D

  // slot % const: -12 % 5 = -2; -2 + 'G' = 'E'
  c.assign("x", literal::s8(-12));
  c.assign("z", c.mod("x", literal::s8(5)));
  c.writeOut(c.add("z", literal::s8('G'))); // E

  // slot % const: 12 % -5 = 2; 2 + 'D' = 'F'
  c.assign("x", literal::s8(12));
  c.assign("z", c.mod("x", literal::s8(-5)));
  c.writeOut(c.add("z", literal::s8('D'))); // F

  // const % slot: -12 % 5 = -2; -2 + 'I' = 'G'
  c.assign("x", literal::s8(5));
  c.assign("z", c.mod(literal::s8(-12), "x"));
  c.writeOut(c.add("z", literal::s8('I'))); // G

  // const % slot: 12 % -5 = 2; 2 + 'F' = 'H'
  c.assign("x", literal::s8(-5));
  c.assign("z", c.mod(literal::s8(12), "x"));
  c.writeOut(c.add("z", literal::s8('F'))); // H

  c.returnFromFunction();
} c.endFunction();

TEST_END
