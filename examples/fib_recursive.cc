#include <iostream>
#include "acus/assembler/assembler.h"
using namespace acus;


int main() try {
  Assembler c;

  c.program("fib", "main").begin(); {

    auto i8 = ts::i8();
					    
    c.function("main").begin(); {
      c.declareLocal("f", i8);
      
      c.block("entry").begin(); {
	c.callFunction("fib", "end").into("f").arg(literal::i8(12)).done();
      } c.endBlock();

      c.block("end").begin(); {
	c.writeOut("f");
	c.returnFromFunction();
      } c.endBlock();

    } c.endFunction();

    c.function("fib")
      .param("n", i8)
      .ret(i8)
      .begin();
    {
      c.declareLocal("f1", i8);
      c.declareLocal("f2", i8);
      
      c.block("entry").begin(); {
	// if (n <= 1) return n;
	c.branchIf(c.le("n", literal::i8(1)), "done", "recurse");
      } c.endBlock();


      c.block("done").begin(); {
	c.returnFromFunction("n");
      } c.endBlock();


      c.block("recurse").begin(); {
	// fib(n - 1)
	auto n_minus_1 = c.sub("n", literal::i8(1));
	c.callFunction("fib", "recurse2").into("f1").arg(n_minus_1).done();
      } c.endBlock();

      c.block("recurse2").begin(); {
	// fib(n - 2)
	auto n_minus_2 = c.sub("n", literal::i8(2));
	c.callFunction("fib", "end").into("f2").arg(n_minus_2).done();
      } c.endBlock();

      c.block("end").begin(); {
	// return fib(n-1) + fib(n-2)
	c.returnFromFunction(c.add("f1", "f2"));
      } c.endBlock();

    } c.endFunction();


  } c.endProgram();
 
  std::cout << c.brainfuck("fib") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
