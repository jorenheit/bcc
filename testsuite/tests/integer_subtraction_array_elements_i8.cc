// Subtract i8 array elements using sub and subAssign
// Expected: GDA

TEST_BEGIN

auto array3 = ts::array(ts::i8(), 3);

c.function("main").begin(); {
  c.declareLocal("arr", array3);
  c.declareLocal("z", ts::i8());

  c.assign(c.arrayElement("arr", 0), literal::i8('G'));
  c.assign(c.arrayElement("arr", 1), literal::i8(3));

  auto x = c.arrayElement("arr", 0);
  auto y = c.arrayElement("arr", 1);

  c.write(x);                 // G
  c.subAssign(x, y);             // arr[0] -= arr[1]
  c.write(x);                 // D
  c.assign("z", c.sub(x, y));    // z = arr[0] - arr[1]
  c.write("z");               // A

  c.returnFromFunction();
} c.endFunction();

TEST_END
