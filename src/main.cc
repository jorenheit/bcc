#include <iostream>
#include "compiler.h"

int main() {
  Compiler c;
  c.setEntryPoint("main");

  c.begin(); {
    c.beginFunction("main"); {
      auto x = c.declareLocal("x");
      c.beginBlock("entry"); {
	c.setToValue('A', x);
	c.writeOut(x);
	c.callFunction("foo", "after");
      } c.endBlock();

      c.beginBlock("after"); {
	c.setToValue('\n', x);
	c.writeOut(x);
	c.returnFromFunction();
      }; c.endBlock();
    } c.endFunction();

    
    c.beginFunction("foo"); {
      auto y = c.declareLocal("y");
      c.beginBlock("entry"); {
       	c.setToValue('B', y);
	c.writeOut(y);
	c.callFunction("bar", "after");
      } c.endBlock();

      c.beginBlock("after"); {
	c.writeOut(y);
	c.returnFromFunction();
      }; c.endBlock();
      
    } c.endFunction();


    c.beginFunction("bar"); {
      auto z = c.declareLocal("z");
      c.beginBlock("entry"); {
       	c.setToValue('C', z);
	c.writeOut(z);
	c.returnFromFunction();
      } c.endBlock();

    } c.endFunction();

  } c.end();


  //  std::cout << c.dumpPrimitives() << "\n\n";
  std::cout << c.dumpBrainfuck() << "\n";
}
