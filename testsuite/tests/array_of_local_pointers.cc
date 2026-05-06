// Tests an array of pointers to locals.
// p[0] points to a, p[1] points to b.
// Reading through both should produce AB, then writing through both changes them to XY.
// Expected: ABXY

TEST_BEGIN

auto i8   = ts::i8();
auto i8p  = ts::pointer(i8);
auto i8pa = ts::array(i8p, 2);

c.function("main").begin(); {
  c.declareLocal("p", i8pa);
  c.declareLocal("a", i8);
  c.declareLocal("b", i8);

  c.assign("a", literal::i8('A'));
  c.assign("b", literal::i8('B'));

  c.assign(c.arrayElement("p", 0), c.addressOf("a"));
  c.assign(c.arrayElement("p", 1), c.addressOf("b"));

  auto p0Deref = c.dereferencePointer(c.arrayElement("p", 0));
  auto p1Deref = c.dereferencePointer(c.arrayElement("p", 1));

  c.writeOut(p0Deref);
  c.writeOut(p1Deref);

  c.assign(p0Deref, literal::i8('X'));
  c.assign(p1Deref, literal::i8('Y'));

  c.writeOut("a");
  c.writeOut("b");

  c.returnFromFunction();
} c.endFunction();

TEST_END
