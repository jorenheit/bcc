// Pass the  string "Hello World" anonymously.
// Expected: "Hello World"

Compiler c;
auto &ts = c.typeSystem();

c.setEntryPoint("main");

std::string str = "Hello World";

auto string = ts.string(str.size());
  
c.begin(); {
  c.beginFunction("main"); {
    c.beginBlock("entry"); {
      c.callFunction("print", "return", values::string(ts, str));
    } c.endBlock();

    c.beginBlock("return"); {
      c.returnFromFunction();
    } c.endBlock();
      
  } c.endFunction();

  c.beginFunction("print", ts.voidT(),
		  "s", string); {
    c.beginBlock("entry"); {
      c.writeOut("s");
      c.returnFromFunction();
    } c.endBlock();
      
  } c.endFunction();

    
} c.end();

return c.dumpBrainfuck();
