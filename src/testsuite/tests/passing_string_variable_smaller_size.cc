// Pass "Hello World" anonymously to a function that expects a bigger one
// Expected: "Hello World"

Compiler c;
auto &ts = c.typeSystem();

c.setEntryPoint("main");

std::string str = "Hello World";

auto smallString = ts.string(str.size());
auto bigString = ts.string(str.size() * 2);
  
c.begin(); {
  c.beginFunction("main"); {
    c.beginBlock("entry"); {
      c.callFunction("print", "return", values::List{
	  values::value(smallString, str)
	});
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
