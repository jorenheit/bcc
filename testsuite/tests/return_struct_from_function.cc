// Return an entire struct object from a function
// Expected: "QR"

TEST_BEGIN
auto point = c.defineStruct("Point")("x", TypeSystem::i8(),
				     "y", TypeSystem::i8());

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
			    
auto sig = TypeSystem::function(point);
c.beginFunction("makePoint", sig); {
  c.declareLocal("p", point);
  c.beginBlock("entry"); {
    auto x = c.structField("p", "x");
    auto y = c.structField("p", "y");

    c.assign(x, values::i8('Q'));
    c.assign(y, values::i8('R'));
    c.returnFromFunction("p");
  } c.endBlock();
} c.endFunction();

TEST_END
