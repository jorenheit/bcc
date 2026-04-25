// Subtract i8 struct fields using sub and subAssign
// Expected: GDA

TEST_BEGIN

auto pairT = c.defineStruct("Pair")("x", TypeSystem::i8(),
                                    "y", TypeSystem::i8());

c.beginFunction("main"); {
  c.declareLocal("p", pairT);
  c.declareLocal("z", TypeSystem::i8());

  c.beginBlock("entry"); {
    auto x = c.structField("p", "x");
    auto y = c.structField("p", "y");

    c.assign(x, values::i8('G'));
    c.assign(y, values::i8(3));

    c.writeOut(x);              // G
    c.subAssign(x, y);          // p.x -= p.y
    c.writeOut(x);              // D
    c.assign("z", c.sub(x, y)); // z = p.x - p.y
    c.writeOut("z");            // A

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
