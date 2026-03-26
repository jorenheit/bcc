// Convert i8 to i16
// Expected: "AA"

Compiler c;
c.setEntryPoint("main");

std::string str = "Hello World";

c.begin(); {
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
    
} c.end();

return c.dumpBrainfuck();
