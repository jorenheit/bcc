// Subtract u16 struct fields using sub and subAssign
// Expected: GADAAA

TEST_BEGIN

auto pairT = ts::defineStruct("Pair16").field("x", ts::u16()).field("y", ts::u16()).done();

c.function("main").begin(); {
  c.declareLocal("p", pairT);
  c.declareLocal("z", ts::u16());

  auto x = c.structField("p", "x");
  auto y = c.structField("p", "y");

  c.assign(x, literal::u16(CAT('G', 'A')));
  c.assign(y, literal::u16(3));

  c.write(x);              // GA
  c.subAssign(x, y);          // p.x -= p.y
  c.write(x);              // DA
  c.assign("z", c.sub(x, y)); // z = p.x - p.y
  c.write("z");            // AA

  c.returnFromFunction();
} c.endFunction();

TEST_END
