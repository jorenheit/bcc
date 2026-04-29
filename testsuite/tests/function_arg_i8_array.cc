// Tests pass-by-value of an i8 array argument, verifying full array copy and correct element ordering in caller and callee.
// Expected: ABCDABCD

TEST_BEGIN
auto array4 = ts::array(ts::i8(), 4);

c.function("main").begin(); {
  c.declareLocal("x", array4);
       
  c.block("entry").begin(); {
    auto x0 = c.arrayElement("x", 0);
    auto x1 = c.arrayElement("x", 1);
    auto x2 = c.arrayElement("x", 2);
    auto x3 = c.arrayElement("x", 3);

    c.assign(x0, literal::i8('A'));
    c.assign(x1, literal::i8('B'));
    c.assign(x2, literal::i8('C'));
    c.assign(x3, literal::i8('D'));

    c.writeOut("x");
    c.callFunction("foo", "after_foo").arg("x").done();
  } c.endBlock();

  c.block("after_foo").begin(); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();


c.function("foo").param("x", array4).ret(ts::void_t()).begin(); {
  c.block("entry").begin(); {
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
