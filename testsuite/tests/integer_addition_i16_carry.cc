// Add u16 integers with carry into the high byte
// Expected: EACBAC

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::u16());
  c.declareLocal("y", ts::u16());
  c.declareLocal("z", ts::u16());

  c.assign("x", literal::u16(CAT('E', 'A'))); // low='D', high='A'
  c.assign("y", literal::u16(254));

  c.write("x");                    // EA
  c.addAssign("x", "y");              // EA + 254 -> CB
  c.write("x");                    // CB
  c.assign("z", c.add("x", "y"));     // CB + 254 -> AC
  c.write("z");                    // AC

  c.returnFromFunction();
} c.endFunction();

TEST_END
