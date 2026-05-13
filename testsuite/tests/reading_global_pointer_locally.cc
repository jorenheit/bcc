// Tests reading through a pointer to a global variable in the same function.
// Expected: GG

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);

c.declareGlobal("g", i8);

c.function("main").begin(); {
  c.referGlobals({"g"});
  c.declareLocal("p", i8p);

  c.assign("g", literal::i8('G'));
  c.assign("p", c.addressOf("g"));

  auto pDeref = c.dereferencePointer("p");
  c.write("g");
  c.write(pDeref);

  c.returnFromFunction();
} c.endFunction();

TEST_END
