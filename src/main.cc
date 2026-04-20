#include <iostream>
#include "compiler.h"

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Compiler c;
  c.setEntryPoint("main");

  c.begin(); {
    auto i8 = TypeSystem::i8();
    c.declareGlobal("x", i8);
      
    c.beginFunction("main"); {
      c.referGlobals({"x"});
      
      c.beginBlock("entry"); {
	c.assign("x", values::i8(0));
	c.setNextBlock("loop");
      } c.endBlock();

      c.beginBlock("loop"); {
	auto args = c.constructFunctionArguments("x");
	c.callFunction("incAndPrint", "loopEnd", args, "x");
      } c.endBlock();

      c.beginBlock("loopEnd"); {
	c.branchIf("x", "loop", "end");	
      } c.endBlock();

      c.beginBlock("end"); {
	c.returnFromFunction();
      } c.endBlock();
      
    } c.endFunction();

    auto sig = c.constructFunctionSignature(i8, "x", i8);
    c.beginFunction("incAndPrint", sig); {
      c.beginBlock("entry"); {
      	c.addAssign("x", values::i8(1));
	c.writeOut("x");
	c.returnFromFunction("x");
      } c.endBlock();
    } c.endFunction();
    
  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
