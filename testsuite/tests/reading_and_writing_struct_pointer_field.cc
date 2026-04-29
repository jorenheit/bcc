// Tests a struct containing a pointer field passed to a function.
// foo reads through s.p, then writes through it. main observes the updated pointee.
// Expected: AAX

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);

auto holder = ts::defineStruct("Holder").field("p", i8p).done();

c.beginFunction("main"); {
  c.declareLocal("s", holder);
  c.declareLocal("x", i8);

  c.beginBlock("entry"); {
    c.assign("x", literal::i8('A'));
    c.assign(c.structField("s", "p"), c.addressOf("x"));

    c.callFunction("foo", "after").arg("s").done();
  } c.endBlock();

  c.beginBlock("after"); {
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto sig = ts::function().ret(ts::void_t()).param(holder).done();
c.beginFunction("foo", sig, {"s"}); {
  c.beginBlock("entry"); {
    auto p = c.structField("s", "p");
    auto pDeref = c.dereferencePointer(p);

    c.writeOut(pDeref);
    c.assign(pDeref, literal::i8('X'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
