// Convert i8 to i16
// Expected: "AA"

Compiler c;
auto &ts = c.typeSystem();

c.setEntryPoint("main");

std::string str = "Hello World";

c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("x", ts.i8());
    c.declareLocal("y", ts.i16());
    
    c.beginBlock("entry"); {
      c.assign(values::Var("x"), values::value(ts.i8(), 'A'));
      c.writeOut("x");
      c.assign(values::Var("y"), values::Var("x"));
      c.writeOut("y");
      c.returnFromFunction();
    } c.endBlock();
      
  } c.endFunction();
    
} c.end();

return c.dumpBrainfuck();
