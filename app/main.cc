#include <iostream>
#include "acus.h"

using namespace acus::api;

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Assembler c;

  c.program("test", "main").begin(); {

    c.function("main").begin(); {
      c.declareLocal("x", ts::i16());
      c.declareLocal("y", ts::i8());
      c.declareLocal("z", ts::i16());

      c.assign("x", literal::i16(0x8282));
      c.assign("y", literal::i8(2));

      c.divAssign("x", "y");           // 0x8282 / 2 = 0x4141 -> AA
      c.writeOut("x");                 // AA

      c.assign("x", literal::i16(0x8686));
      c.assign("z", c.div("x", "y"));  // 0x8686 / 2 = 0x4343 -> CC
      c.writeOut("z");                 // CC

      c.returnFromFunction();
    } c.endFunction();


    
  } c.endProgram();

  std::cout << c.brainfuck("test") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
