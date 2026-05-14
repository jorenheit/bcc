// Subtract u8 struct fields using sub and subAssign
// Expected: GDA

TEST_BEGIN

auto pairT = ts::defineStruct("Pair").field("x", ts::u8()).field("y", ts::u8()).done();

c.function("main").begin(); {
  c.declareLocal("p", pairT);
  c.declareLocal("z", ts::u8());

  auto x = c.structField("p", "x");
  auto y = c.structField("p", "y");

  c.assign(x, literal::u8('G'));
  c.assign(y, literal::u8(3));

  c.write(x);              // G
  c.subAssign(x, y);          // p.x -= p.y
  c.write(x);              // D
  c.assign("z", c.sub(x, y)); // z = p.x - p.y
  c.write("z");            // A

  c.returnFromFunction();
} c.endFunction();

TEST_END
