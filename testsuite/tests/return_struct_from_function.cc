// Return an entire struct object from a function
// Expected: "QR"

TEST_BEGIN
auto point = ts::defineStruct("Point")("x", ts::i8(),
				     "y", ts::i8());

c.beginFunction("main"); {
  c.declareLocal("s", point);

  c.beginBlock("entry"); {
    c.callFunction("makePoint", "after_makePoint", "s")();
  } c.endBlock();

  c.beginBlock("after_makePoint"); {
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();
			    
auto sig = ts::function(point)();
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
