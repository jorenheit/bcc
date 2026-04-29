// Subtract i8 struct fields using sub and subAssign
// Expected: GDA

TEST_BEGIN

auto pairT = ts::defineStruct("Pair").field("x", ts::i8()).field("y", ts::i8()).done();

c.function("main").begin(); {
  c.declareLocal("p", pairT);
  c.declareLocal("z", ts::i8());

  c.block("entry").begin(); {
    auto x = c.structField("p", "x");
    auto y = c.structField("p", "y");

    c.assign(x, literal::i8('G'));
    c.assign(y, literal::i8(3));

    c.writeOut(x);              // G
    c.subAssign(x, y);          // p.x -= p.y
    c.writeOut(x);              // D
    c.assign("z", c.sub(x, y)); // z = p.x - p.y
    c.writeOut("z");            // A

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
