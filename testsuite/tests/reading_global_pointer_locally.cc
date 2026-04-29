// Tests reading through a pointer to a global variable in the same function.
// Expected: GG

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

    auto pDeref = c.dereferencePointer("p");
    c.writeOut("g");
    c.writeOut(pDeref);

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
