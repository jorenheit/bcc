// Tests returning a global value into a caller local while preserving the global for later reads.
// GG
TEST_BEGIN

c.declareGlobal("g", TypeSystem::i8());

c.beginFunction("main"); {
  c.declareLocal("x", TypeSystem::i8());
      
  c.beginBlock("entry"); {
    c.callFunction("foo", "after_foo", {}, "x");
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.writeOut("x");
    c.callFunction("bar", "after_bar");
  } c.endBlock();

  c.beginBlock("after_bar"); {
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

auto fooSig = TypeSystem::function(TypeSystem::i8());
c.beginFunction("foo", fooSig); {
  c.referGlobals({"g"});
  c.beginBlock("entry"); {
    c.assign("g", values::i8('G'));
    c.returnFromFunction("g");
  } c.endBlock();
} c.endFunction();

c.beginFunction("bar"); {
  c.referGlobals({"g"});
  c.beginBlock("entry"); {
    c.writeOut("g");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
