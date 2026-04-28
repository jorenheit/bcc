// Tests recursive pointer relocation through array -> struct -> pointer.
// foo reads through both pointer fields, then writes through both.
// Expected: ABXY

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);

auto holder = ts::defineStruct("Holder")("p", i8p);
auto holders = ts::array(holder, 2);

c.beginFunction("main"); {
  c.declareLocal("arr", holders);
  c.declareLocal("a", i8);
  c.declareLocal("b", i8);

  c.beginBlock("entry"); {
    c.assign("a", literal::i8('A'));
    c.assign("b", literal::i8('B'));

    c.assign(c.structField(c.arrayElement("arr", 0), "p"), c.addressOf("a"));
    c.assign(c.structField(c.arrayElement("arr", 1), "p"), c.addressOf("b"));

    c.callFunction("foo", "after")("arr");
  } c.endBlock();

  c.beginBlock("after"); {
    c.writeOut("a");
    c.writeOut("b");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();


auto sig = ts::function(ts::voidT())(holders);
c.beginFunction("foo", sig, {"arr"}); {
  c.beginBlock("entry"); {
    auto p0 = c.structField(c.arrayElement("arr", 0), "p");
    auto p1 = c.structField(c.arrayElement("arr", 1), "p");

    auto d0 = c.dereferencePointer(p0);
    auto d1 = c.dereferencePointer(p1);

    c.writeOut(d0);
    c.writeOut(d1);

    c.assign(d0, literal::i8('X'));
    c.assign(d1, literal::i8('Y'));

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
