// Tests recursive self-calls with multiple i8 parameters, stressing repeated frame creation and parameter re-passing.
// Expect: ABCABCABCABC...
TEST_BEGIN
c.beginFunction("main"); {
  c.beginBlock("entry"); {
    
    c.callFunction("foo", "after_foo").arg(literal::i8('A')).arg(literal::i8('B')).arg(literal::i8('C')).done();
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = ts::function().ret(ts::void_t()).param(ts::i8()).param(ts::i8()).param(ts::i8()).done();
c.beginFunction("foo", fooSig, {"x", "y", "z"}); {
  c.beginBlock("entry"); {
    c.writeOut("x");
    c.writeOut("y");
    c.writeOut("z");

    c.callFunction("foo", "after_recurse").arg("x").arg("y").arg("z").done();
  } c.endBlock();

  c.beginBlock("after_recurse"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();
TEST_END
