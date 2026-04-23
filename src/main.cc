#include <iostream>
#include "compiler.h"

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Compiler c;
  c.setEntryPoint("main");

  c.begin(); {

    auto i8 = TypeSystem::i8();
    auto i16 = TypeSystem::i16();
    auto arrayT = TypeSystem::array(i8, 4);
    
    c.beginFunction("main"); {
      c.declareLocal("x", i8);
      c.declareLocal("y", i16);

      c.beginBlock("entry"); {

	c.assign("x", values::i8(0));
	c.assign("y", values::i16(1));
	c.writeOut(c.lnand("x", "y"));
	
	
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
