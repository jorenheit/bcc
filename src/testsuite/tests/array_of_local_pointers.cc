// Tests an array of pointers to locals.
// p[0] points to a, p[1] points to b.
// Reading through both should produce AB, then writing through both changes them to XY.
// Expected: ABXY

TEST_BEGIN

auto i8   = TypeSystem::i8();
auto i8p  = TypeSystem::pointer(i8);
auto i8pa = TypeSystem::array(i8p, 2);

c.beginFunction("main"); {
  c.declareLocal("p", i8pa);
  c.declareLocal("a", i8);
  c.declareLocal("b", i8);

  c.beginBlock("entry"); {
    c.assign("a", values::i8('A'));
    c.assign("b", values::i8('B'));

    c.assign(c.arrayElement("p", 0), c.addressOf("a")); 
    c.assign(c.arrayElement("p", 1), c.addressOf("b"));

    auto p0Deref = c.dereferencePointer(c.arrayElement("p", 0));
    auto p1Deref = c.dereferencePointer(c.arrayElement("p", 1));

    c.writeOut(p0Deref);
    c.writeOut(p1Deref);

    c.assign(p0Deref, values::i8('X'));
    c.assign(p1Deref, values::i8('Y'));

    c.writeOut("a");
    c.writeOut("b");

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
