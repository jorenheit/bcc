#include <iostream>
#include "compiler.h"

int main() try {
  Compiler c;
  auto &ts = c.typeSystem();

  c.setEntryPoint("main");
  
  c.begin(); {
    c.beginFunction("main"); {
      c.declareLocal("x", ts.i8());
      c.declareLocal("y", ts.i16());

      c.beginBlock("entry"); {
	c.callFunction("foo", "next", values::List{
	    values::Var("x"),
	    values::Var("y")
	  });
      } c.endBlock();

      c.beginBlock("next"); {
	c.returnFromFunction();
      } c.endBlock();
      
    } c.endFunction();

    c.beginFunction("foo", ts.voidT(),
		    "x", ts.i8(),
		    "y", ts.i16()); {
      c.beginBlock("entry"); {
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
