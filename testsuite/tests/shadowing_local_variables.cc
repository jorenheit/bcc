// Tests shadowing of local variables in enclosed nested scopes
// Expected: "ABXCDA"

TEST_BEGIN
c.function("main").begin(); {
  c.declareLocal("x", ts::u8());

  c.assign("x", literal::u8('A'));
  c.write("x");
  c.scope().begin(); {
    c.declareLocal("x", ts::array(ts::u8(), 2));
    c.assign("x", literal::array(ts::array(ts::u8(), 2)).push(literal::u8('B')).push(literal::u8('X')).done());
    c.write("x");
    c.scope().begin(); {
      c.declareLocal("x", ts::u8());
      c.declareLocal("y", ts::u8());
      c.assign("x", literal::u8('C'));
      c.assign("y", literal::u8('D'));
      c.write("x");
      c.write("y");
    } c.endScope();

  } c.endScope();

  c.write("x");
  c.returnFromFunction();
} c.endFunction();

TEST_END
