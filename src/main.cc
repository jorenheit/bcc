#include <iostream>
#include "compiler.h"

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Compiler c;
  c.setEntryPoint("main");

  c.begin(); {
    auto i8 = TypeSystem::i8();
    auto i16 = TypeSystem::i16();
    auto voidT = TypeSystem::voidT();
    
    c.beginFunction("main"); {
      c.declareLocal("x", i8);
      c.declareLocal("y", i16);

      c.beginBlock("entry"); {
	c.assign("x", values::i8('A'));
	c.assign("y", values::i16('B'));
	c.callFunction("foo", "end",
		       c.constructFunctionArguments("x", "y"));
      } c.endBlock();

      c.beginBlock("end"); {
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    auto fooType = TypeSystem::function(voidT, i8, i16);
    c.beginFunction("foo", fooType, {"x", "y"}); {
      c.beginBlock("entry"); {
	c.writeOut("x");
	c.writeOut("y");
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
