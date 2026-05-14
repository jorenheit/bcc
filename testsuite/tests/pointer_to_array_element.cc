// Tests taking a pointer to an array element, reading through it,
// then writing through it.
// arr[1] starts as 'B', is read through p, then changed to 'X' through p.
// Expected: BAXC

TEST_BEGIN

auto u8  = ts::u8();
auto u8p = ts::pointer(u8);
auto arr3 = ts::array(u8, 3);

c.function("main").begin(); {
  c.declareLocal("p", u8p);
  c.declareLocal("arr", arr3);

  c.assign(c.arrayElement("arr", 0), literal::u8('A'));
  c.assign(c.arrayElement("arr", 1), literal::u8('B'));
  c.assign(c.arrayElement("arr", 2), literal::u8('C'));

  c.assign("p", c.addressOf(c.arrayElement("arr", 1)));

  auto pDeref = c.dereferencePointer("p");
  c.write(pDeref); // B

  c.assign(pDeref, literal::u8('X'));

  c.write(c.arrayElement("arr", 0)); // A
  c.write(c.arrayElement("arr", 1)); // X
  c.write(c.arrayElement("arr", 2)); // C

  c.returnFromFunction();
} c.endFunction();

TEST_END
