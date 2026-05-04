#include <iostream>
#include "acus.h"

using namespace acus::api;

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Assembler c;

  c.program("test", "main").begin(); {
    c.function("main").begin(); {
      c.declareLocal("x", ts::s16());
      c.declareLocal("y", ts::s16());
      c.declareLocal("f", ts::s16());
      
      c.block("entry").begin(); {

	// ORRU
    
	// signBit(-1) = 1; 1 + 'P' = 'Q'
	c.assign("x", literal::s16(-1));
	c.assign("f", c.signBit("x"));
	c.writeOut(c.add("f", literal::s16(CAT('P', 'Q')))); // Q

	// signBit(0) = 0; 0 + 'R' = 'R'
	c.assign("x", literal::s16(0));
	c.assign("f", c.signBit("x"));
	c.writeOut(c.add("f", literal::s16(CAT('R', 'R')))); // R

	// signBitAssign(-32768): x becomes 1; CAT('R', 'T') + 1 = CAT('S', 'T') -> ST
	c.assign("x", literal::s16(-32768));
	c.signBitAssign("x");
	c.addAssign("x", literal::s16(CAT('R', 'S')));
	c.writeOut("x"); // ST

	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();


  } c.endProgram();

  std::cout << c.brainfuck("test") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
