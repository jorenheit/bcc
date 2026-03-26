// Store the string "Hello World" in a variable, then pass it to the print function
// Expected: "Hello World"

Compiler c;

c.setEntryPoint("main");

std::string str = "Hello World";

auto string = TypeSystem::string(str.size());
  
c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("s", string);

    c.beginBlock("entry"); {
      c.assign("s", values::string(str));
      c.callFunction("print", "return", "s");
    } c.endBlock();

    c.beginBlock("return"); {
      c.returnFromFunction();
    } c.endBlock();
      
  } c.endFunction();

  c.beginFunction("print", TypeSystem::voidT(),
		  "s", string); {
    c.beginBlock("entry"); {
      c.writeOut("s");
      c.returnFromFunction();
    } c.endBlock();
      
  } c.endFunction();

    
} c.end();

return c.dumpBrainfuck();
