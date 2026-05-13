// Copying a pointer preserves the old target when the original is adjusted
// Expected: AB

TEST_BEGIN

auto i8   = ts::i8();
auto i8p  = ts::pointer(i8);
auto arrT = ts::array(i8, 2);

c.function("main").begin(); {
  c.declareLocal("p", i8p);
  c.declareLocal("q", i8p);
  c.declareLocal("arr", arrT);

  c.assign(c.arrayElement("arr", 0), literal::i8('A'));
  c.assign(c.arrayElement("arr", 1), literal::i8('B'));

  c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
  c.assign("q", "p");

  c.addAssign("p", literal::i16(1));

  c.write(c.dereferencePointer("q")); // A
  c.write(c.dereferencePointer("p")); // B

  c.returnFromFunction();
} c.endFunction();

TEST_END
