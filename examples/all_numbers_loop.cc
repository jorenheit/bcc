#include <iostream>
#include "acus/assembler/assembler.h"
using namespace acus;


int main() try {
  Assembler c;

  c.program("hello", "main").begin(); {

    c.function("main").begin(); {
      c.declareLocal("x", ts::u8());
      c.assign("x", literal::u8(0));

      c.label("print");
      c.print("x");
      c.write(literal::u8('\n'));
      c.addAssign("x", literal::u8(1));
      c.jumpIf(c.eq("x", literal::u8(0)), "done", "print");

      c.label("done");
      c.returnFromFunction();

    } c.endFunction();

  } c.endProgram();
 
  std::cout << c.brainfuck("hello") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
