#include <iostream>
#include "acus.h"

using namespace acus::api;

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Assembler c;

  c.program("test", "main").begin(); {
    c.function("main").begin(); {
      c.declareLocal("x", ts::s8());
      
      c.block("entry").begin(); {
	c.assign("x", literal::s8(-43));
	c.print("x");
	
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();


  } c.endProgram();

  std::cout << c.brainfuck("test") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
