// Write through an adjusted pointer into an i8 array
// Expected: ABXD

TEST_BEGIN

auto i8   = ts::i8();
auto i8p  = ts::pointer(i8);
auto arrT = ts::array(i8, 4);

c.function("main").begin(); {
  c.declareLocal("p", i8p);
  c.declareLocal("arr", arrT);

  c.assign(c.arrayElement("arr", 0), literal::i8('A'));
  c.assign(c.arrayElement("arr", 1), literal::i8('B'));
  c.assign(c.arrayElement("arr", 2), literal::i8('C'));
  c.assign(c.arrayElement("arr", 3), literal::i8('D'));

  c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
  c.addAssign("p", literal::i16(2));

  auto pDeref = c.dereferencePointer("p");
  c.assign(pDeref, literal::i8('X'));

  c.writeOut(c.arrayElement("arr", 0)); // A
  c.writeOut(c.arrayElement("arr", 1)); // B
  c.writeOut(c.arrayElement("arr", 2)); // X
  c.writeOut(c.arrayElement("arr", 3)); // D

  c.returnFromFunction();
} c.endFunction();

TEST_END
