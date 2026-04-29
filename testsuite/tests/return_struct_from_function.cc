// Return an entire struct object from a function
// Expected: "QR"

TEST_BEGIN
auto point = ts::defineStruct("Point").field("x", ts::i8()).field("y", ts::i8()).done();

c.function("main").begin(); {
  c.declareLocal("s", point);

  c.block("entry").begin(); {
    c.callFunction("makePoint", "after_makePoint").into("s").done();
  } c.endBlock();

  c.block("after_makePoint").begin(); {
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();
			    
c.function("makePoint").ret(point).begin(); {
  c.declareLocal("p", point);
  c.block("entry").begin(); {
    auto x = c.structField("p", "x");
    auto y = c.structField("p", "y");

    c.assign(x, literal::i8('Q'));
    c.assign(y, literal::i8('R'));
    c.returnFromFunction("p");
  } c.endBlock();
} c.endFunction();

TEST_END
