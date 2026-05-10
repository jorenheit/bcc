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

      // -1 < 0 -> true
      c.assign("x", literal::s16(-1));
      c.assign("y", literal::s16(0));
      c.writeOut(c.add(c.lt("x", "y"), literal::i8('A'))); // B

      // // 0 < -1 -> false
      c.writeOut(c.add(c.lt("y", "x"), literal::i8('A'))); // A

      // -300 < -20 -> true
      c.assign("x", literal::s16(-300));
      c.assign("y", literal::s16(-20));
      c.writeOut(c.add(c.lt("x", "y"), literal::i8('A'))); // B

      // -20 < -300 -> false
      c.writeOut(c.add(c.lt("y", "x"), literal::i8('A'))); // A

      // 32767 < -32768 -> false
      c.assign("x", literal::s16(32767));
      c.assign("y", literal::s16(-32768));
      c.writeOut(c.add(c.lt("x", "y"), literal::i8('A'))); // A

      // -32768 < 32767 -> true
      c.writeOut(c.add(c.lt("y", "x"), literal::i8('A'))); // B

      // // -1 > 0 -> false
      // c.assign("x", literal::s16(-1));
      // c.assign("y", literal::s16(0));
      // c.writeOut(c.add(c.gt("x", "y"), literal::i8('A'))); // A

      // // 0 > -1 -> true
      // c.writeOut(c.add(c.gt("y", "x"), literal::i8('A'))); // B

      // // -300 <= -300 -> true
      // c.assign("x", literal::s16(-300));
      // c.assign("y", literal::s16(-300));
      // c.writeOut(c.add(c.le("x", "y"), literal::i8('A'))); // B

      // // -300 >= -300 -> true
      // c.writeOut(c.add(c.ge("x", "y"), literal::i8('A'))); // B

      // // -300 >= -20 -> false
      // c.assign("x", literal::s16(-300));
      // c.assign("y", literal::s16(-20));
      // c.writeOut(c.add(c.ge("x", "y"), literal::i8('A'))); // A

      // // -20 >= -300 -> true
      // c.writeOut(c.add(c.ge("y", "x"), literal::i8('A'))); // B

      c.returnFromFunction();
    } c.endFunction();

  } c.endProgram();

  std::cout << c.brainfuck("test") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
