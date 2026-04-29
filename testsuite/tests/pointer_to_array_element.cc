// Tests taking a pointer to an array element, reading through it,
// then writing through it.
// arr[1] starts as 'B', is read through p, then changed to 'X' through p.
// Expected: BAXC

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);
auto arr3 = ts::array(i8, 3);

c.function("main").begin(); {
  c.declareLocal("p", i8p);
  c.declareLocal("arr", arr3);

  c.block("entry").begin(); {
    c.assign(c.arrayElement("arr", 0), literal::i8('A'));
    c.assign(c.arrayElement("arr", 1), literal::i8('B'));
    c.assign(c.arrayElement("arr", 2), literal::i8('C'));

    c.assign("p", c.addressOf(c.arrayElement("arr", 1)));

    auto pDeref = c.dereferencePointer("p");
    c.writeOut(pDeref); // B

    c.assign(pDeref, literal::i8('X'));

    c.writeOut(c.arrayElement("arr", 0)); // A
    c.writeOut(c.arrayElement("arr", 1)); // X
    c.writeOut(c.arrayElement("arr", 2)); // C

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
