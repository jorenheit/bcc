// Tests returning a global value into a caller local while preserving the global for later reads.
// GG
TEST_BEGIN

c.declareGlobal("g", ts::i8());

c.function("main").begin(); {
  c.declareLocal("x", ts::i8());

  c.callFunction("foo").into("x").done();
  c.writeOut("x");
  c.callFunction("bar").done();
  c.returnFromFunction();
} c.endFunction();

c.function("foo").ret(ts::i8()).begin(); {
  c.referGlobals({"g"});
  c.assign("g", literal::i8('G'));
  c.returnFromFunction("g");
} c.endFunction();

c.function("bar").begin(); {
  c.referGlobals({"g"});
  c.writeOut("g");
  c.returnFromFunction();
} c.endFunction();

TEST_END
