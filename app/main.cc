#include <iostream>
#include "bfc/compiler/compiler.h"

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Compiler c;
  c.setEntryPoint("main");

  c.begin(); {

    // auto i8 = TypeSystem::i8();

    // c.beginFunction("main"); {
    //   c.declareLocal("n", i8);
    //   c.declareLocal("i", i8);
    //   c.declareLocal("a", i8);
    //   c.declareLocal("b", i8);
    //   c.declareLocal("next", i8);

    //   c.beginBlock("entry"); {
    // 	c.assign("n", values::i8(12));

    // 	// fib(0) = 0, fib(1) = 1
    // 	c.assign("i", values::i8(0));
    // 	c.assign("a", values::i8(0));
    // 	c.assign("b", values::i8(1));

    // 	c.setNextBlock("loop_check");
    //   } c.endBlock();

    //   c.beginBlock("loop_check"); {
    // 	// while (i <= n)
    // 	c.branchIf(c.le("i", "n"), "loop_body", "done");
    //   } c.endBlock();

    //   c.beginBlock("loop_body"); {
    // 	// Output current Fibonacci number.
    // 	c.writeOut("a");

    // 	// next = a + b
    // 	c.assign("next", c.add("a", "b"));

    // 	// a = b
    // 	c.assign("a", "b");

    // 	// b = next
    // 	c.assign("b", "next");

    // 	// i++
    // 	c.addAssign("i", values::i8(1));
    // 	c.setNextBlock("loop_check");
    //   } c.endBlock();

    //   c.beginBlock("done"); {
    // 	c.returnFromFunction();
    //   } c.endBlock();

      
    // } c.endFunction();

    auto i8 = TypeSystem::i8();
    c.defineStruct("Point")("x", i8,
			    "y", i8);
    

    c.beginFunction("main"); {
      c.declareLocal("f", i8);
      
      c.beginBlock("entry"); {
	c.callFunction("fib", "end", "f")(values::i8(12));
      } c.endBlock();

      c.beginBlock("end"); {
	c.writeOut("f");
	c.returnFromFunction();
      } c.endBlock();

    } c.endFunction();


    // fib(n: i8) -> i8
    auto fibType = TypeSystem::function(i8, i8);

    c.beginFunction("fib", fibType, {"n"}); {
      c.declareLocal("f1", i8);
      c.declareLocal("f2", i8);
      
      c.beginBlock("entry"); {
	// if (n <= 1) return n;
	c.branchIf(c.le("n", values::i8(1)), "done", "recurse");
      } c.endBlock();


      c.beginBlock("done"); {
	c.returnFromFunction("n");
      } c.endBlock();


      c.beginBlock("recurse"); {
	// fib(n - 1)
	auto n_minus_1 = c.sub("n", values::i8(1));
	c.callFunction("fib", "recurse2", "f1")(n_minus_1);
      } c.endBlock();

      c.beginBlock("recurse2"); {
	// fib(n - 2)
	auto n_minus_2 = c.sub("n", values::i8(2));
	c.callFunction("fib", "end", "f2")(n_minus_2);
      } c.endBlock();

      c.beginBlock("end"); {
	// return fib(n-1) + fib(n-2)
	c.returnFromFunction(c.add("f1", "f2"));
      } c.endBlock();

    } c.endFunction();


  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
