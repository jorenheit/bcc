
// Tests returning an i8 array from a callee into a caller local array.
// Expected: ABCDABCD

TEST_BEGIN
auto array4 = ts::array(ts::i8(), 4);
    
c.function("main").begin(); {
  c.declareLocal("x", array4);
      
  c.block("entry").begin(); {
    c.callFunction("foo", "after_foo").into("x").done();
  } c.endBlock();

  c.block("after_foo").begin(); {
    c.writeOut(c.arrayElement("x", 0));
    c.writeOut(c.arrayElement("x", 1));
    c.writeOut(c.arrayElement("x", 2));
    c.writeOut(c.arrayElement("x", 3));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("foo").ret(array4).begin(); {
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
    c.returnFromFunction("x");
  } c.endBlock();
} c.endFunction();

TEST_END
