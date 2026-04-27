#include <iostream>
#include "acus/builder/builder.h"
using namespace acus;


int main() try {
  Builder c;
  c.setEntryPoint("main");

  c.begin(); {

    auto i8 = TypeSystem::i8();
					    
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

    auto fibType = TypeSystem::function(i8, {i8});
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
