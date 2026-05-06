// Tests assigning a function return value directly into a global and reading it back in a later call.
// Expected result: YY

TEST_BEGIN
c.declareGlobal("g", ts::i8());

c.function("main").begin(); {
  c.referGlobals({"g"});

  c.callFunction("foo").into("g").done();
  c.writeOut("g");
  c.callFunction("bar").done();
  c.returnFromFunction();
} c.endFunction();

c.function("foo").ret(ts::i8()).begin(); {
  c.declareLocal("y", ts::i8());
  c.assign("y", literal::i8('Y'));
  c.returnFromFunction("y");
} c.endFunction();

c.function("bar").begin(); {
  c.referGlobals({"g"});
  c.writeOut("g");
  c.returnFromFunction();
} c.endFunction();

TEST_END
