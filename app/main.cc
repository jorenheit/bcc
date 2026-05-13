#include <iostream>
#include "acus.h"

using namespace acus::api;

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Assembler c;

  auto i8 = ts::i8();
  auto s8 = ts::s8();
  auto i16 = ts::i16();
  auto s16 = ts::s16();

  c.program("test", "main").begin(); {

    c.function("main").begin(); {
      c.declareLocal("x", s16);

      c.assign("x", literal::s16(-300));
      c.print("x");
      

      c.returnFromFunction();
    } c.endFunction();



 } c.endProgram();

  std::cout << c.brainfuck("test") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
