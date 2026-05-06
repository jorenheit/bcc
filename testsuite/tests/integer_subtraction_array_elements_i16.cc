// Subtract i16 array elements using sub and subAssign
// Expected: GADAAA

TEST_BEGIN

auto array3 = ts::array(ts::i16(), 3);

c.function("main").begin(); {
  c.declareLocal("arr", array3);
  c.declareLocal("z", ts::i16());

  c.assign(c.arrayElement("arr", 0), literal::i16(CAT('G', 'A')));
  c.assign(c.arrayElement("arr", 1), literal::i16(3));

  auto x = c.arrayElement("arr", 0);
  auto y = c.arrayElement("arr", 1);

  c.writeOut(x);              // GA
  c.subAssign(x, y);          // arr[0] -= arr[1]
  c.writeOut(x);              // DA
  c.assign("z", c.sub(x, y)); // z = arr[0] - arr[1]
  c.writeOut("z");            // AA

  c.returnFromFunction();
} c.endFunction();

TEST_END
