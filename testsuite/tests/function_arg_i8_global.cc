// Tests passing a global i8 by value as a function argument, verifying that modifying the parameter does not mutate the global.
// Expected: GHG

TEST_BEGIN

c.declareGlobal("g", ts::i8());
    
c.beginFunction("main"); {
  c.referGlobals({"g"});
      
  c.beginBlock("entry"); {
    c.assign("g", literal::i8('G'));
    c.callFunction("foo", "after_foo")("g");
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.writeOut("g");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = ts::function(ts::voidT())(ts::i8());
c.beginFunction("foo", fooSig, {"arg1"}); {
  c.beginBlock("entry"); {
    c.writeOut("arg1");
    c.assign("arg1", literal::i8('H'));
    c.writeOut("arg1");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
