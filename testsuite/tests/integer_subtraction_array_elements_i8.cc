// Subtract i8 array elements using sub and subAssign
// Expected: GDA

TEST_BEGIN

auto array3 = TypeSystem::array(TypeSystem::i8(), 3);

c.beginFunction("main"); {
  c.declareLocal("arr", array3);
  c.declareLocal("z", TypeSystem::i8());

  c.beginBlock("entry"); {
    c.assign(c.arrayElement("arr", 0), values::i8('G'));
    c.assign(c.arrayElement("arr", 1), values::i8(3));

    auto x = c.arrayElement("arr", 0);
    auto y = c.arrayElement("arr", 1);

    c.writeOut(x);                 // G
    c.subAssign(x, y);             // arr[0] -= arr[1]
    c.writeOut(x);                 // D
    c.assign("z", c.sub(x, y));    // z = arr[0] - arr[1]
    c.writeOut("z");               // A

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
