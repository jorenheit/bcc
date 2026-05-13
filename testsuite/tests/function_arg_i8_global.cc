// Tests passing a global i8 by value as a function argument, verifying that modifying the parameter does not mutate the global.
// Expected: GHG

TEST_BEGIN

c.declareGlobal("g", ts::i8());

c.function("main").begin(); {
  c.referGlobals({"g"});

  c.assign("g", literal::i8('G'));
  c.callFunction("foo").arg("g").done();
  c.write("g");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("arg1", ts::i8()).ret(ts::void_t()).begin(); {
  c.write("arg1");
  c.assign("arg1", literal::i8('H'));
  c.write("arg1");
  c.returnFromFunction();
} c.endFunction();

TEST_END
