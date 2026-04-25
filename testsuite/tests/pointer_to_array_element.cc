// Tests taking a pointer to an array element, reading through it,
// then writing through it.
// arr[1] starts as 'B', is read through p, then changed to 'X' through p.
// Expected: BAXC

TEST_BEGIN

auto i8  = TypeSystem::i8();
auto i8p = TypeSystem::pointer(i8);
auto arr3 = TypeSystem::array(i8, 3);

c.beginFunction("main"); {
  c.declareLocal("p", i8p);
  c.declareLocal("arr", arr3);

  c.beginBlock("entry"); {
    c.assign(c.arrayElement("arr", 0), values::i8('A'));
    c.assign(c.arrayElement("arr", 1), values::i8('B'));
    c.assign(c.arrayElement("arr", 2), values::i8('C'));

    c.assign("p", c.addressOf(c.arrayElement("arr", 1)));

    auto pDeref = c.dereferencePointer("p");
    c.writeOut(pDeref); // B

    c.assign(pDeref, values::i8('X'));

    c.writeOut(c.arrayElement("arr", 0)); // A
    c.writeOut(c.arrayElement("arr", 1)); // X
    c.writeOut(c.arrayElement("arr", 2)); // C

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
