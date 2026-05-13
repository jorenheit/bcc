// Pass a pointer to a global variable to a function that outputs its pointee. Then change its value and pass it again.
// Expected: "GHX"

TEST_BEGIN

types::TypeHandle i8 = ts::i8();
types::TypeHandle i8p = ts::pointer(ts::i8());


c.declareGlobal("g", i8);

c.function("main").begin(); {
  c.referGlobals({"g"});
  c.declareLocal("pg", i8p);
  c.declareLocal("x", i8);

  c.assign("pg", c.addressOf("g"));
  c.assign("g", literal::i8('G'));
  c.assign("x", literal::i8('X'));
  c.callFunction("foo").arg("pg").done();
  c.assign("g", literal::i8('H'));

  c.callFunction("foo").arg("pg").done();
  c.write("x");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("p", i8p).ret(ts::void_t()).begin(); {
  auto pDeref = c.dereferencePointer("p");
  c.write(pDeref);
  c.returnFromFunction();
} c.endFunction();


TEST_END
