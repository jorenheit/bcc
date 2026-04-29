// Tests returning a global value into a caller local while preserving the global for later reads.
// GG
TEST_BEGIN

c.declareGlobal("g", ts::i8());

c.function("main").begin(); {
  c.declareLocal("x", ts::i8());
      
  c.block("entry").begin(); {
    c.callFunction("foo", "after_foo").into("x").done();
  } c.endBlock();

  c.block("after_foo").begin(); {
    c.writeOut("x");
    c.callFunction("bar", "after_bar").done();
  } c.endBlock();

  c.block("after_bar").begin(); {
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

c.function("foo").ret(ts::i8()).begin(); {
  c.referGlobals({"g"});
  c.block("entry").begin(); {
    c.assign("g", literal::i8('G'));
    c.returnFromFunction("g");
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
