// Tests correct argument ordering when constants and i8 locals are interleaved in a function call.
// Expect: AZBY

TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("a", TypeSystem::i8());
  c.declareLocal("b", TypeSystem::i8());

  c.beginBlock("entry"); {
    c.assign("a", values::i8('Z'));
    c.assign("b", values::i8('Y'));

    c.callFunction("foo", "after_foo")(values::i8('A'), "a", values::i8('B'), "b");
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = TypeSystem::function(TypeSystem::voidT(), TypeSystem::i8(), TypeSystem::i8(), TypeSystem::i8(), TypeSystem::i8());
c.beginFunction("foo", fooSig, {"p0", "p1", "p2", "p3"}); {
  c.beginBlock("entry"); {
    c.writeOut("p0");
    c.writeOut("p1");
    c.writeOut("p2");
    c.writeOut("p3");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
