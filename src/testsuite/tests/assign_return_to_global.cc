// Tests assigning a function return value directly into a global and reading it back in a later call.
// Expected result: YY

TEST_BEGIN
c.declareGlobal("g", TypeSystem::i8());

c.beginFunction("main"); {
  c.referGlobals({"g"});
      
  c.beginBlock("entry"); {
    c.callFunction("foo", "after_foo", {}, "g");
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.writeOut("g");
    c.callFunction("bar", "after_bar");
  } c.endBlock();

  c.beginBlock("after_bar"); {
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

auto fooSig = TypeSystem::function(TypeSystem::i8());
c.beginFunction("foo", fooSig); {
  c.declareLocal("y", TypeSystem::i8());
  c.beginBlock("entry"); {
    c.assign("y", values::i8('Y'));
    c.returnFromFunction("y");
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
