// Store the string "Hello World" in a variable, then pass it to the print function that expects a bigger one
// Expected: "Hello World"

Compiler c;
auto &ts = c.typeSystem();

c.setEntryPoint("main");

std::string str = "Hello World";

auto smallString = ts.string(str.size());
auto bigString = ts.string(str.size() * 2);
  
c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("s", smallString);

    c.beginBlock("entry"); {
      c.assign("s", values::string(ts, str));
      c.callFunction("print", "return", "s");
    } c.endBlock();

    c.beginBlock("return"); {
      c.returnFromFunction();
    } c.endBlock();
      
  } c.endFunction();

  c.beginFunction("print", ts.voidT(),
		  "s", bigString); {
    c.beginBlock("entry"); {
      c.writeOut("s");
      c.returnFromFunction();
    } c.endBlock();
      
  } c.endFunction();

    
} c.end();

return c.dumpBrainfuck();
