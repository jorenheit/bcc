// Tests coherence between named global access and pointer-based global access across a call.
// main writes g by name, foo reads it through a pointer and writes through that pointer,
// then main reads g by name again.
// Expected: AGX

TEST_BEGIN

auto i8  = TypeSystem::i8();
auto i8p = TypeSystem::pointer(i8);

c.declareGlobal("g", i8);

c.beginFunction("main"); {
  c.referGlobals({"g"});
  c.declareLocal("p", i8p);

  c.beginBlock("entry"); {
    c.assign("g", values::i8('A'));
    c.assign("p", c.addressOf("g"));

    c.callFunction("foo", "after")("p");
  } c.endBlock();

  c.beginBlock("after"); {
    c.writeOut("g");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto sig = TypeSystem::function(TypeSystem::voidT(), i8p);
c.beginFunction("foo", sig, {"p"}); {
  c.referGlobals({"g"});
  c.beginBlock("entry"); {
    auto pDeref = c.dereferencePointer("p");
    c.writeOut(pDeref);
    c.assign(pDeref, values::i8('X'));
    c.writeOut("g");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
