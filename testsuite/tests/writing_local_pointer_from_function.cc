// Tests writing through a pointer that points to a caller local and is passed to a function.
// foo writes 'Y' through p, so x changes from 'X' to 'Y'.
// Expected: YY

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);

c.function("main").begin(); {
  c.declareLocal("p", i8p);
  c.declareLocal("x", i8);

  c.assign("x", literal::i8('X'));
  c.assign("p", c.addressOf("x"));
  c.callFunction("foo").arg("p").done();
  c.writeOut("x");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("p", i8p).ret(ts::void_t()).begin(); {
  auto pDeref = c.dereferencePointer("p");
  c.assign(pDeref, literal::i8('Y'));
  c.writeOut(pDeref);
  c.returnFromFunction();
} c.endFunction();

TEST_END
