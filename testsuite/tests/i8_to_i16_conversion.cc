// Convert i8 to i16
// Expected: "AA"

TEST_BEGIN

std::string str = "Hello World";

c.beginFunction("main"); {
  c.declareLocal("x", TypeSystem::i8());
  c.declareLocal("y", TypeSystem::i16());
    
  c.beginBlock("entry"); {
    c.assign("x", values::i8('A'));
    c.writeOut("x");
    c.assign("y", "x");
    c.writeOut("y");
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

TEST_END
