// Tests writing through an array of pointers passed to a function.
// foo reads both pointees, writes through both pointers, main observes the updates.
// Expected: ABXY

TEST_BEGIN

auto i8   = ts::i8();
auto i8p  = ts::pointer(i8);
auto i8pa = ts::array(i8p, 2);

c.function("main").begin(); {
  c.declareLocal("p", i8pa);
  c.declareLocal("a", i8);
  c.declareLocal("b", i8);

  c.block("entry").begin(); {
    c.assign("a", literal::i8('A'));
    c.assign("b", literal::i8('B'));

    c.assign(c.arrayElement("p", 0), c.addressOf("a"));
    c.assign(c.arrayElement("p", 1), c.addressOf("b"));

    c.callFunction("foo", "after").arg("p").done();
  } c.endBlock();

  c.block("after").begin(); {
    c.writeOut("a");
    c.writeOut("b");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("foo").param("p", i8pa).ret(ts::void_t()).begin(); {
  c.block("entry").begin(); {
    auto p0Deref = c.dereferencePointer(c.arrayElement("p", 0));
    auto p1Deref = c.dereferencePointer(c.arrayElement("p", 1));

    c.writeOut(p0Deref);
    c.writeOut(p1Deref);

    c.assign(p0Deref, literal::i8('X'));
    c.assign(p1Deref, literal::i8('Y'));

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
