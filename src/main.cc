#include <iostream>
#include "compiler.h"

int main() try {
  Compiler c;
  auto &ts = c.typeSystem();

  c.setEntryPoint("main");

  auto string = ts.string(15);
  
  c.begin(); {
    c.beginFunction("main"); {
      c.declareLocal("s", string);

      c.beginBlock("entry"); {
	//	c.assign("s", values::value(string, "Hello World\n"));
	c.callFunction("print", "return", values::List{
	    //  values::Var("s")
	    values::value(string, "Hello World\n")	    
	  });
      } c.endBlock();

      c.beginBlock("return"); {
      	c.returnFromFunction();
      } c.endBlock();
      
    } c.endFunction();

    c.beginFunction("print", ts.voidT(),
		    "s", ts.string(20)); {
      c.beginBlock("entry"); {
	c.writeOut("s");
	c.returnFromFunction();
      } c.endBlock();
      
    } c.endFunction();

    
  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
