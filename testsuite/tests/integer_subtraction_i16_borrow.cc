// Subtract u16 integers with borrow into the high byte
// Expected: AB?A=A

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u16());
  c.declareLocal("y", ts::u16());
  c.declareLocal("z", ts::u16());

  c.assign("x", literal::u16(CAT('A', 'C'))); // low='A', high='B'
  c.assign("y", literal::u16(254));

  c.write("x");                    // AC
  c.subAssign("x", "y");              //
  c.write("x");                    // CB
  c.assign("z", c.sub("x", "y"));     //
  c.write("z");                    // EA

  c.returnFromFunction();
} c.endFunction();

TEST_END

