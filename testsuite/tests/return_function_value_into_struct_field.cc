// Store the return-value of a function into a struct-field
// Expected: "A"

TEST_BEGIN

auto point = ts::defineStruct("Point").field("x", ts::i8()).field("y", ts::i8()).done();

c.beginFunction("main"); {
  c.declareLocal("s", point);

  c.beginBlock("entry"); {
    auto x = c.structField("s", "x");
    auto y = c.structField("s", "y");

    c.assign(x, literal::i8('A'));
    c.callFunction("makeZ", "after_makeZ").into(y).done();
  } c.endBlock();

  c.beginBlock("after_makeZ"); {
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto sig = ts::function().ret(ts::i8()).done();
c.beginFunction("makeZ", sig); {
  c.beginBlock("entry"); {
    c.returnFromFunction(literal::i8('Z'));
  } c.endBlock();
} c.endFunction();

TEST_END
