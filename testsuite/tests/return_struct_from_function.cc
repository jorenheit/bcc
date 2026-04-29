// Return an entire struct object from a function
// Expected: "QR"

TEST_BEGIN
auto point = ts::defineStruct("Point").field("x", ts::i8()).field("y", ts::i8()).done();

c.beginFunction("main"); {
  c.declareLocal("s", point);

  c.beginBlock("entry"); {
    c.callFunction("makePoint", "after_makePoint").into("s").done();
  } c.endBlock();

  c.beginBlock("after_makePoint"); {
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();
			    
auto sig = ts::function().ret(point).done();
c.beginFunction("makePoint", sig); {
  c.declareLocal("p", point);
  c.beginBlock("entry"); {
    auto x = c.structField("p", "x");
    auto y = c.structField("p", "y");

    c.assign(x, literal::i8('Q'));
    c.assign(y, literal::i8('R'));
    c.returnFromFunction("p");
  } c.endBlock();
} c.endFunction();

TEST_END
