// Tests referenced global array access across a call, including element-wise writes in caller and reads in callee.
// Expected output: ABCD

TEST_BEGIN

c.declareGlobal("x", ts::array(ts::i8(), 10));
    
c.beginFunction("main"); {
  c.referGlobals({"x"});
      
  c.beginBlock("entry"); {
    auto x0 = c.arrayElement("x", 0);
    auto x1 = c.arrayElement("x", 1);
    auto x2 = c.arrayElement("x", 2);
    auto x3 = c.arrayElement("x", 3);
	
    c.assign(x0, literal::i8('A'));
    c.assign(x1, literal::i8('B'));
    c.assign(x2, literal::i8('C'));
    c.assign(x3, literal::i8('D'));

    c.callFunction("foo", "after_foo")();
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("foo"); {
  c.referGlobals({"x"});
      
  c.beginBlock("entry"); {
    auto x0 = c.arrayElement("x", literal::i8(0));
    auto x1 = c.arrayElement("x", literal::i8(1));
    auto x2 = c.arrayElement("x", literal::i8(2));
    auto x3 = c.arrayElement("x", literal::i8(3));
	
    c.writeOut(x0);
    c.writeOut(x1);
    c.writeOut(x2);
    c.writeOut(x3);

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
