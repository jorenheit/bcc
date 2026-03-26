// Pass "Hello World" anonymously to a function that expects a bigger one
// Expected: "Hello World"

Compiler c;

c.setEntryPoint("main");

std::string str = "Hello World";

  
c.begin(); {
  c.beginFunction("main"); {
    c.beginBlock("entry"); {
      c.callFunction("print", "return", values::string(str));
    } c.endBlock();

    c.beginBlock("return"); {
      c.returnFromFunction();
    } c.endBlock();
      
  } c.endFunction();

  c.beginFunction("print", TypeSystem::voidT(),
		  "s", TypeSystem::string(str.size() * 2)); {
    c.beginBlock("entry"); {
      c.writeOut("s");
      c.returnFromFunction();
    } c.endBlock();
      
  } c.endFunction();

    
} c.end();

return c.dumpBrainfuck();
