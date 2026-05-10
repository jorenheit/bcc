#include <iostream>
#include "acus.h"

using namespace acus::api;

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Assembler c;

  c.program("test", "main").begin(); {

    c.function("main").begin(); {
      c.declareLocal("x", ts::s16());
      c.assign("x", literal::s16(-300));
      c.print("x");
      c.writeOut(literal::i8('\n'));
      c.print(c.cast("x", ts::i8()));
      c.writeOut(literal::i8('\n'));
      c.returnFromFunction();
    } c.endFunction();

  } c.endProgram();

  std::cout << c.brainfuck("test") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
