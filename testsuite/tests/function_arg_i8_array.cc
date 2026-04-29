// Tests pass-by-value of an i8 array argument, verifying full array copy and correct element ordering in caller and callee.
// Expected: ABCDABCD

TEST_BEGIN
auto array4 = ts::array(ts::i8(), 4);

c.beginFunction("main"); {
  c.declareLocal("x", array4);
       
  c.beginBlock("entry"); {
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

  c.beginBlock("after_foo"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();


auto fooSig = ts::function().ret(ts::void_t()).param(array4).done();
c.beginFunction("foo", fooSig, {"x"}); {
  c.beginBlock("entry"); {
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
