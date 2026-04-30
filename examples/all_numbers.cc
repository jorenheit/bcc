#include <iostream>
#include "acus/assembler/assembler.h"
using namespace acus;


int main() try {
  Assembler c;

  c.program("hello", "main").begin(); {

    c.function("main").begin(); {
      c.declareLocal("x", ts::i8());

      c.block("entry").begin(); {
	for (int i = 0; i != 256; ++i) {
	  c.assign("x", literal::i8(i));
	  c.writeOut("x");
	}

	c.returnFromFunction();
      } c.endBlock();
      
    } c.endFunction();

  } c.endProgram();
 
  std::cout << c.brainfuck("hello") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
