#include <iostream>
#include "acus/assembler/assembler.h"
using namespace acus;


int main() try {
  Assembler c;

  c.program("hello", "main").begin(); {

    c.function("main").begin(); {
      c.declareLocal("x", ts::i8());
      c.assign("x", literal::i8(0));

      c.label("print");
      c.print("x");
      c.write(literal::i8('\n'));
      c.addAssign("x", literal::i8(1));
      c.jumpIf(c.eq("x", literal::i8(0)), "done", "print");

      c.label("done");
      c.returnFromFunction();

    } c.endFunction();

  } c.endProgram();
 
  std::cout << c.brainfuck("hello") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
