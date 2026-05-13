#include <iostream>
#include "acus/assembler/assembler.h"
using namespace acus;

int main() try {
  Assembler c;

  c.program("fib", "main").begin(); {

    auto i8 = ts::i8();

    c.function("main").begin(); {
      c.declareLocal("n", i8);
      c.declareLocal("i", i8);
      c.declareLocal("a", i8);
      c.declareLocal("b", i8);
      c.declareLocal("next", i8);

      c.assign("n", literal::i8(12)); // Number of iterations

      // fib(0) = 0, fib(1) = 1
      c.assign("i", literal::i8(0));
      c.assign("a", literal::i8(0));
      c.assign("b", literal::i8(1));

      c.label("loop_check");
      c.jumpIf(c.le("i", "n"), "loop_body", "done");
      c.label("loop_body");

      // Output current Fibonacci number.
      c.print("a");
      c.print(literal::string(", "));
      // next = a + b
      c.assign("next", c.add("a", "b"));
      // a = b
      c.assign("a", "b");
      // b = next
      c.assign("b", "next");
      // i++
      c.addAssign("i", literal::i8(1));
      c.jump("loop_check");

      c.label("done");
      c.returnFromFunction();
    } c.endFunction();

    
  } c.endProgram();

  std::cout << c.brainfuck("fib") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
