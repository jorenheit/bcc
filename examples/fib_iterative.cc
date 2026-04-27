#include <iostream>
#include "acus/builder/builder.h"
using namespace acus;

int main() try {
  Builder c;
  c.setEntryPoint("main");

  c.begin(); {

    auto i8 = TypeSystem::i8();

    c.beginFunction("main"); {
      c.declareLocal("n", i8);
      c.declareLocal("i", i8);
      c.declareLocal("a", i8);
      c.declareLocal("b", i8);
      c.declareLocal("next", i8);

      c.beginBlock("entry"); { 
	c.assign("n", values::i8(12)); // Number of iterations

	// fib(0) = 0, fib(1) = 1
	c.assign("i", values::i8(0));
	c.assign("a", values::i8(0));
	c.assign("b", values::i8(1));

	c.setNextBlock("loop_check");
      } c.endBlock();

      c.beginBlock("loop_check"); {
	// while (i <= n)
	c.branchIf(c.le("i", "n"), "loop_body", "done");
      } c.endBlock();

      c.beginBlock("loop_body"); {
	// Output current Fibonacci number.
	c.writeOut("a");

	// next = a + b
	c.assign("next", c.add("a", "b"));

	// a = b
	c.assign("a", "b");

	// b = next
	c.assign("b", "next");

	// i++
	c.addAssign("i", values::i8(1));
	c.setNextBlock("loop_check");
      } c.endBlock();

      c.beginBlock("done"); {
	c.returnFromFunction();
      } c.endBlock();

    } c.end();

    std::cout << c.dumpBrainfuck() << '\n';
  } catch (std::exception const &e) {
    std::cerr << e.what() << '\n';
  }
