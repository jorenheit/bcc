// Return a value from a function directly into an array element selected by a runtime index.
// Expected: "ABCZ"

TEST_BEGIN

auto array4 = ts::array(ts::i8(), 4);

c.beginFunction("main"); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", ts::i8());

  c.beginBlock("entry"); {
    c.assign("arr", literal::array(ts::i8())(literal::i8('A'),
						    literal::i8('B'),
						    literal::i8('C'),
						    literal::i8('D')));
    c.assign("idx", literal::i8(3));

    auto dest = c.arrayElement("arr", "idx");
    c.callFunction("makeZ", "after_makeZ", dest)();
  } c.endBlock();

  c.beginBlock("after_makeZ"); {
    c.writeOut("arr");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto sig = ts::function(ts::i8())();
c.beginFunction("makeZ", sig); {
  c.beginBlock("entry"); {
    c.returnFromFunction(literal::i8('Z'));
  } c.endBlock();
} c.endFunction();

TEST_END
