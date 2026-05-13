// Filename: integer_literals_i8.cc
// Literal RHS tests for i8 operators and assign-operators
// Expected: ABCDEABCDE

TEST_BEGIN

auto i8 = ts::i8();

c.function("main").begin(); {
  c.declareLocal("x", i8);

  // add with literal -> 'A'
  c.assign("x", literal::i8(64));
  c.write(c.add("x", literal::i8(1)));

  // sub with literal -> 'B'
  c.assign("x", literal::i8(67));
  c.write(c.sub("x", literal::i8(1)));

  // div with literal, mapped to 'C'
  c.assign("x", literal::i8(99));
  c.write(c.add(c.div("x", literal::i8(3)), literal::i8(34)));

  // mul with literal -> 'D'
  c.assign("x", literal::i8(34));
  c.write(c.mul("x", literal::i8(2)));

  // mod with literal, mapped to 'E'
  c.assign("x", literal::i8(100));
  c.write(c.add(c.mod("x", literal::i8(34)), literal::i8(37)));

  // addAssign with literal -> 'A'
  c.assign("x", literal::i8(64));
  c.addAssign("x", literal::i8(1));
  c.write("x");

  // subAssign with literal -> 'B'
  c.assign("x", literal::i8(67));
  c.subAssign("x", literal::i8(1));
  c.write("x");

  // divAssign with literal, mapped to 'C'
  c.assign("x", literal::i8(99));
  c.divAssign("x", literal::i8(3));
  c.write(c.add("x", literal::i8(34)));

  // mulAssign with literal -> 'D'
  c.assign("x", literal::i8(34));
  c.mulAssign("x", literal::i8(2));
  c.write("x");

  // modAssign with literal, mapped to 'E'
  c.assign("x", literal::i8(100));
  c.modAssign("x", literal::i8(34));
  c.write(c.add("x", literal::i8(37)));

  c.returnFromFunction();
} c.endFunction();

TEST_END
