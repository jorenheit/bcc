// Tests shadowing of local variables in enclosed nested scopes
// Expected: "ABXCDA"

TEST_BEGIN
c.function("main").begin(); {
  c.declareLocal("x", ts::i8());

  c.assign("x", literal::i8('A'));
  c.write("x");
  c.scope().begin(); {
    c.declareLocal("x", ts::array(ts::i8(), 2));
    c.assign("x", literal::array(ts::array(ts::i8(), 2)).push(literal::i8('B')).push(literal::i8('X')).done());
    c.write("x");
    c.scope().begin(); {
      c.declareLocal("x", ts::i8());
      c.declareLocal("y", ts::i8());
      c.assign("x", literal::i8('C'));
      c.assign("y", literal::i8('D'));
      c.write("x");
      c.write("y");
    } c.endScope();

  } c.endScope();

  c.write("x");
  c.returnFromFunction();
} c.endFunction();

TEST_END
