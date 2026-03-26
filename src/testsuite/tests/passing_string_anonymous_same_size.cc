// Pass the  string "Hello World" anonymously.
// Expected: "Hello World"

Compiler c;
c.setEntryPoint("main");

std::string str = "Hello World";

auto string = TypeSystem::string(str.size());
  
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
		  "s", string); {
    c.beginBlock("entry"); {
      c.writeOut("s");
      c.returnFromFunction();
    } c.endBlock();
      
  } c.endFunction();

    
} c.end();

return c.dumpBrainfuck();
