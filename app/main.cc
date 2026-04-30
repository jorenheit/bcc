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
      c.declareLocal("y", i8);
      
      c.block("entry").begin(); {
	c.assign("x", literal::i8('X'));
	c.callFunction("foo", "end").into("y").arg("x").done();
      } c.endBlock();

      c.block("end").begin(); {
	c.writeOut("x");
	c.writeOut("y");
	c.returnFromFunction();
      } c.endBlock();
      
    } c.endFunction();

    c.function("foo").param("x", i8).ret(i8).begin(); {
      c.block("entry").begin(); {
	c.returnFromFunction("x");
      } c.endBlock();
    } c.endFunction();

  } c.endProgram();


  c.program("fib", "main").begin(); {
    auto i8 = ts::i8();

    c.function("main").begin(); {
      c.declareLocal("n", i8);
      c.declareLocal("i", i8);
      c.declareLocal("a", i8);
      c.declareLocal("b", i8);
      c.declareLocal("next", i8);

      c.block("entry").begin(); { 
	c.assign("n", literal::i8(12)); // Number of iterations

	// fib(0) = 0, fib(1) = 1
	c.assign("i", literal::i8(0));
	c.assign("a", literal::i8(0));
	c.assign("b", literal::i8(1));

	c.setNextBlock("loop_check");
      } c.endBlock();

      c.block("loop_check").begin(); {
	// while (i <= n)
	c.branchIf(c.le("i", "n"), "loop_body", "done");
      } c.endBlock();

      c.block("loop_body").begin(); {
	// Output current Fibonacci number.
	c.writeOut("a");

	// next = a + b
	c.assign("next", c.add("a", "b"));

	// a = b
	c.assign("a", "b");

	// b = next
	c.assign("b", "next");

	// i++
	c.addAssign("i", literal::i8(1));
	c.setNextBlock("loop_check");
      } c.endBlock();

      c.block("done").begin(); {
	c.returnFromFunction();
      } c.endBlock();

    } c.endFunction();

  } c.endProgram();
  
  std::cout << c.brainfuck("fib") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
