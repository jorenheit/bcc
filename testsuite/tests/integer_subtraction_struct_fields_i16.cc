// Subtract i16 struct fields using sub and subAssign
// Expected: GADAAA

TEST_BEGIN

auto pairT = ts::defineStruct("Pair16")("x", ts::i16(),
                                    "y", ts::i16());

c.beginFunction("main"); {
  c.declareLocal("p", pairT);
  c.declareLocal("z", ts::i16());

  c.beginBlock("entry"); {
    auto x = c.structField("p", "x");
    auto y = c.structField("p", "y");

    c.assign(x, literal::i16(CAT('G', 'A')));
    c.assign(y, literal::i16(3));

    c.writeOut(x);              // GA
    c.subAssign(x, y);          // p.x -= p.y
    c.writeOut(x);              // DA
    c.assign("z", c.sub(x, y)); // z = p.x - p.y
    c.writeOut("z");            // AA

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
