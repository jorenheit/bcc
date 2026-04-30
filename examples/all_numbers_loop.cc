#include <iostream>
#include "acus/assembler/assembler.h"
using namespace acus;


int main() try {
  Assembler c;

  c.program("hello", "main").begin(); {

    c.function("main").begin(); {
      c.declareLocal("x", ts::i8());

      c.block("entry").begin(); {
	c.assign("x", literal::i8(0));
	c.setNextBlock("print");
      } c.endBlock();

      c.block("print").begin(); {
	c.writeOut("x");
	c.addAssign("x", literal::i8(1));
	c.branchIf(c.eq("x", literal::i8(0)), "done", "print");
      } c.endBlock();

      c.block("done").begin(); {
	c.returnFromFunction();
      } c.endBlock();
      
    } c.endFunction();

  } c.endProgram();
 
  std::cout << c.brainfuck("hello") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
