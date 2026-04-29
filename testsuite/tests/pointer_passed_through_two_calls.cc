// Tests repeated pointer-depth adjustment across two call boundaries.
// main creates a pointer to x, foo forwards it to bar, and bar reads/writes through it.
// Expected: AX

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);

c.function("main").begin(); {
  c.declareLocal("p", i8p);
  c.declareLocal("x", i8);

  c.block("entry").begin(); {
    c.assign("x", literal::i8('A'));
    c.assign("p", c.addressOf("x"));
    c.callFunction("foo", "after").arg("p").done();
  } c.endBlock();

  c.block("after").begin(); {
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("foo").param("p", i8p).ret(ts::void_t()).begin(); {
  c.block("entry").begin(); {
    c.callFunction("bar", "return").arg("p").done();
  } c.endBlock();

  c.block("return").begin(); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("bar").param("p", i8p).ret(ts::void_t()).begin(); {
  c.block("entry").begin(); {
    auto pDeref = c.dereferencePointer("p");
    c.writeOut(pDeref);
    c.assign(pDeref, literal::i8('X'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
