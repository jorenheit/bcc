// Tests recursive self-calls with multiple i8 parameters, stressing repeated frame creation and parameter re-passing.
// Expect: ABCABCABCABC...
TEST_BEGIN
c.beginFunction("main"); {
  c.beginBlock("entry"); {
    
    c.callFunction("foo", "after_foo")(literal::i8('A'),
					     literal::i8('B'),
					     literal::i8('C'));
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = ts::function(ts::voidT())(ts::i8(), ts::i8(), ts::i8());
c.beginFunction("foo", fooSig, {"x", "y", "z"}); {
  c.beginBlock("entry"); {
    c.writeOut("x");
    c.writeOut("y");
    c.writeOut("z");

    c.callFunction("foo", "after_recurse")("x", "y", "z");
  } c.endBlock();

  c.beginBlock("after_recurse"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();
TEST_END
