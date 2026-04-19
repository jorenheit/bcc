// Copying a pointer preserves the old target when the original is adjusted
// Expected: AB

TEST_BEGIN

auto i8   = TypeSystem::i8();
auto i8p  = TypeSystem::pointer(i8);
auto arrT = TypeSystem::array(i8, 2);

c.beginFunction("main"); {
  c.declareLocal("p", i8p);
  c.declareLocal("q", i8p);
  c.declareLocal("arr", arrT);

  c.beginBlock("entry"); {
    c.assign(c.arrayElement("arr", 0), values::i8('A'));
    c.assign(c.arrayElement("arr", 1), values::i8('B'));

    c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
    c.assign("q", "p");

    c.addAssign("p", values::i16(1));

    c.writeOut(c.dereferencePointer("q")); // A
    c.writeOut(c.dereferencePointer("p")); // B

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
