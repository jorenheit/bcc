// Tests writing through a pointer to a global variable from inside a function.
// main initializes g to 'G', foo writes 'H' through p, then main prints g.
// Expected: HH

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);

c.declareGlobal("g", i8);

c.function("main").begin(); {
  c.referGlobals({"g"});
  c.declareLocal("p", i8p);

  c.block("entry").begin(); {
    c.assign("g", literal::i8('G'));
    c.assign("p", c.addressOf("g")); 
    c.callFunction("foo", "after").arg("p").done();
  } c.endBlock();

  c.block("after").begin(); {
    c.writeOut("g");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("foo").param("p", i8p).ret(ts::void_t()).begin(); {
  c.referGlobals({"g"});
  c.block("entry").begin(); {
    auto pDeref = c.dereferencePointer("p");
    c.assign(pDeref, literal::i8('H'));
    c.writeOut("g");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
