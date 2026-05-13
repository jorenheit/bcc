// Tests coherence between named global access and pointer-based global access across a call.
// main writes g by name, foo reads it through a pointer and writes through that pointer,
// then main reads g by name again.
// Expected: AGX

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);

c.declareGlobal("g", i8);

c.function("main").begin(); {
  c.referGlobals({"g"});
  c.declareLocal("p", i8p);

  c.assign("g", literal::i8('A'));
  c.assign("p", c.addressOf("g"));

  c.callFunction("foo").arg("p").done();
  c.write("g");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("p", i8p).ret(ts::void_t()).begin(); {
  c.referGlobals({"g"});
  auto pDeref = c.dereferencePointer("p");
  c.write(pDeref);
  c.assign(pDeref, literal::i8('X'));
  c.write("g");
  c.returnFromFunction();
} c.endFunction();

TEST_END
