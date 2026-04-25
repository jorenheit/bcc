// Subtract i16 array elements using sub and subAssign
// Expected: GADAAA

TEST_BEGIN

auto array3 = TypeSystem::array(TypeSystem::i16(), 3);

c.beginFunction("main"); {
  c.declareLocal("arr", array3);
  c.declareLocal("z", TypeSystem::i16());

  c.beginBlock("entry"); {
    c.assign(c.arrayElement("arr", 0), values::i16(CAT('G', 'A')));
    c.assign(c.arrayElement("arr", 1), values::i16(3));

    auto x = c.arrayElement("arr", 0);
    auto y = c.arrayElement("arr", 1);

    c.writeOut(x);              // GA
    c.subAssign(x, y);          // arr[0] -= arr[1]
    c.writeOut(x);              // DA
    c.assign("z", c.sub(x, y)); // z = arr[0] - arr[1]
    c.writeOut("z");            // AA

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
