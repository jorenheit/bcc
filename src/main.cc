#include <iostream>
#include "compiler.h"

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Compiler c;
  c.setEntryPoint("main");

  c.begin(); {
    auto i8 = TypeSystem::i8();
    auto i16 = TypeSystem::i16();
      
    c.beginFunction("main"); {
      c.declareLocal("x", i16);
      c.declareLocal("y", i8);
      
      c.beginBlock("entry"); {
	c.assign("x", values::i16(10));
	c.assign("y", values::i8(100));
	c.writeOut(c.mul("x", "y"));
	c.returnFromFunction();
      } c.endBlock();
      
    } c.endFunction();

    
  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
