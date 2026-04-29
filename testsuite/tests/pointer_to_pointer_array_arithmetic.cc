// Do pointer arithmetic on an array of pointers and dereference twice
// Expected: ACB

TEST_BEGIN

auto i8    = ts::i8();
auto i8p   = ts::pointer(i8);
auto i8pp  = ts::pointer(i8p);
auto ptrArrT = ts::array(i8p, 3);

c.function("main").begin(); {
  c.declareLocal("x", i8);
  c.declareLocal("y", i8);
  c.declareLocal("z", i8);
  c.declareLocal("pp", i8pp);
  c.declareLocal("qq", i8pp);
  c.declareLocal("arr", ptrArrT);

  c.block("entry").begin(); {
    c.assign("x", literal::i8('A'));
    c.assign("y", literal::i8('B'));
    c.assign("z", literal::i8('C'));

    c.assign(c.arrayElement("arr", 0), c.addressOf("x"));
    c.assign(c.arrayElement("arr", 1), c.addressOf("y"));
    c.assign(c.arrayElement("arr", 2), c.addressOf("z"));

    c.assign("pp", c.addressOf(c.arrayElement("arr", 0)));

    auto p0 = c.dereferencePointer("pp");
    c.writeOut(c.dereferencePointer(p0)); // A

    c.assign("qq", c.add("pp", literal::i16(2)));
    auto p2 = c.dereferencePointer("qq");
    c.writeOut(c.dereferencePointer(p2)); // C

    c.subAssign("qq", literal::i16(1));
    auto p1 = c.dereferencePointer("qq");
    c.writeOut(c.dereferencePointer(p1)); // B

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
