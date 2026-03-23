// Store the string "Hello World" in a variable, then pass it to the print function
// Expected: "Hello World"

Compiler c;
auto &ts = c.typeSystem();

c.setEntryPoint("main");

std::string str = "Hello World";

auto string = ts.string(str.size());
  
c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("s", string);

    c.beginBlock("entry"); {
      c.assign("s", values::value(string, str));
      c.callFunction("print", "return", values::List{
	  values::Var("s")
	});
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
