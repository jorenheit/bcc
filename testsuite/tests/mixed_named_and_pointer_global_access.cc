// Tests coherence between named global access and pointer-based global access across a call.
// main writes g by name, foo reads it through a pointer and writes through that pointer,
// then main reads g by name again.
// Expected: AGX

TEST_BEGIN

auto u8  = ts::u8();
auto u8p = ts::pointer(u8);

c.declareGlobal("g", u8);

c.function("main").begin(); {
  c.referGlobals({"g"});
  c.declareLocal("p", u8p);

  c.assign("g", literal::u8('A'));
  c.assign("p", c.addressOf("g"));

  c.callFunction("foo").arg("p").done();
  c.write("g");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("p", u8p).ret(ts::void_t()).begin(); {
  c.referGlobals({"g"});
  auto pDeref = c.dereferencePointer("p");
  c.write(pDeref);
  c.assign(pDeref, literal::u8('X'));
  c.write("g");
  c.returnFromFunction();
} c.endFunction();

TEST_END
