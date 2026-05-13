// Subtract i16 integers with borrow into the high byte
// Expected: AB?A=A

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::i16());
  c.declareLocal("y", ts::i16());
  c.declareLocal("z", ts::i16());

  c.assign("x", literal::i16(CAT('A', 'C'))); // low='A', high='B'
  c.assign("y", literal::i16(254));

  c.write("x");                    // AC
  c.subAssign("x", "y");              //
  c.write("x");                    // CB
  c.assign("z", c.sub("x", "y"));     //
  c.write("z");                    // EA

  c.returnFromFunction();
} c.endFunction();

TEST_END

