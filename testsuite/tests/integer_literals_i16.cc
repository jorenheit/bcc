// Filename: integer_literals_i16.cc
// Literal RHS tests for i16 operators and assign-operators
// Expected: AABBCCDDEEAABBCCDDEE

TEST_BEGIN

auto i16 = ts::i16();

c.function("main").begin(); {
  c.declareLocal("x", i16);

  // add with literal -> "AA"
  c.assign("x", literal::i16(0x4140));
  c.write(c.add("x", literal::i16(1)));

  // sub with literal -> "BB"
  c.assign("x", literal::i16(0x4243));
  c.write(c.sub("x", literal::i16(1)));

  // mul with literal, mapped to "CC"
  c.assign("x", literal::i16(0x1111));
  c.write(c.add(c.mul("x", literal::i16(3)), literal::i16(0x1010)));

  // div with literal, mapped to "DD"
  c.assign("x", literal::i16(0x6363));
  c.write(c.add(c.div("x", literal::i16(3)), literal::i16(0x2323)));

  // mod with literal, mapped to "EE"
  c.assign("x", literal::i16(0x7171));
  c.write(c.add(c.mod("x", literal::i16(0x5050)), literal::i16(0x2424)));

  // addAssign with literal -> "AA"
  c.assign("x", literal::i16(0x4140));
  c.addAssign("x", literal::i16(1));
  c.write("x");

  // subAssign with literal -> "BB"
  c.assign("x", literal::i16(0x4243));
  c.subAssign("x", literal::i16(1));
  c.write("x");

  // mulAssign with literal, mapped to "CC"
  c.assign("x", literal::i16(0x1111));
  c.mulAssign("x", literal::i16(3));
  c.write(c.add("x", literal::i16(0x1010)));

  // divAssign with literal, mapped to "DD"
  c.assign("x", literal::i16(0x6363));
  c.divAssign("x", literal::i16(3));
  c.write(c.add("x", literal::i16(0x2323)));

  // modAssign with literal, mapped to "EE"
  c.assign("x", literal::i16(0x7171));
  c.modAssign("x", literal::i16(0x5050));
  c.write(c.add("x", literal::i16(0x2424)));

  c.returnFromFunction();
} c.endFunction();

TEST_END
