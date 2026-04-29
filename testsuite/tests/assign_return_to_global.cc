// Tests assigning a function return value directly into a global and reading it back in a later call.
// Expected result: YY

TEST_BEGIN
c.declareGlobal("g", ts::i8());

c.function("main").begin(); {
  c.referGlobals({"g"});
      
  c.block("entry").begin(); {
    c.callFunction("foo", "after_foo").into("g").done();
  } c.endBlock();

  c.block("after_foo").begin(); {
    c.writeOut("g");
    c.callFunction("bar", "after_bar").done();
  } c.endBlock();

  c.block("after_bar").begin(); {
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

c.function("foo").ret(ts::i8()).begin(); {
  c.declareLocal("y", ts::i8());
  c.block("entry").begin(); {
    c.assign("y", literal::i8('Y'));
    c.returnFromFunction("y");
  } c.endBlock();
} c.endFunction();

c.function("bar").begin(); {
  c.referGlobals({"g"});
  c.block("entry").begin(); {
    c.writeOut("g");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();
    
TEST_END
