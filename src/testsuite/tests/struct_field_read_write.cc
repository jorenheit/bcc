// Tests writing to struct fields and reading back from them.
// Expected: "ABAB"


TEST_BEGIN

auto point = c.defineStruct("Point",
			    "x", TypeSystem::i8(),
			    "y", TypeSystem::i8());

c.beginFunction("main"); {
  c.declareLocal("s", point);

      
  c.beginBlock("entry"); {
    auto x = c.structField("s", "x");
    auto y = c.structField("s", "y");
      
    c.assign(x, values::i8('A'));
    c.assign(y, values::i8('B'));

    c.writeOut(x);
    c.writeOut(y);
    c.writeOut("s");
	
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

    
TEST_END
