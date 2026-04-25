// Do pointer arithmetic on an i8 array using add, addAssign and subAssign
// Expected: ACBB

TEST_BEGIN

auto i8   = TypeSystem::i8();
auto i8p  = TypeSystem::pointer(i8);
auto arrT = TypeSystem::array(i8, 4);

c.beginFunction("main"); {
  c.declareLocal("p", i8p);
  c.declareLocal("q", i8p);
  c.declareLocal("arr", arrT);

  c.beginBlock("entry"); {
    c.assign(c.arrayElement("arr", 0), values::i8('A'));
    c.assign(c.arrayElement("arr", 1), values::i8('B'));
    c.assign(c.arrayElement("arr", 2), values::i8('C'));
    c.assign(c.arrayElement("arr", 3), values::i8('D'));

    c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
    c.writeOut(c.dereferencePointer("p"));        // A

    c.assign("q", c.add("p", values::i16(2)));
    c.writeOut(c.dereferencePointer("q"));        // C

    c.addAssign("p", values::i16(1));
    c.writeOut(c.dereferencePointer("p"));        // B

    c.subAssign("q", values::i16(1));
    c.writeOut(c.dereferencePointer("q"));        // B

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
