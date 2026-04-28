// Convert i8 to i16
// Expected: "AA"

TEST_BEGIN

std::string str = "Hello World";

c.beginFunction("main"); {
  c.declareLocal("x", ts::i8());
  c.declareLocal("y", ts::i16());
    
  c.beginBlock("entry"); {
    c.assign("x", literal::i8('A'));
    c.writeOut("x");
    c.assign("y", "x");
    c.writeOut("y");
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

TEST_END
