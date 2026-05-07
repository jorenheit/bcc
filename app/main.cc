#include <iostream>
#include "acus.h"

using namespace acus::api;

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Assembler c;

  c.program("test", "main").begin(); {
    auto i8 = ts::i8();

    c.function("main").begin(); {
      c.declareLocal("x", i8);
      c.print("x");
      c.returnFromFunction();
    } c.endFunction();
    
  } c.endProgram();

  std::cout << c.brainfuck("test") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
