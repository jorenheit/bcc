#include <iostream>
#include "acus/assembler/assembler.h"
using namespace acus;


int main() try {
  Assembler c;

  c.program("fib", "main").begin(); {

    auto i8 = ts::i8();
					    
    c.function("main").begin(); {
      c.declareLocal("f", i8);
      c.callFunction("fib").into("f").arg(literal::i8(12)).done();
      c.print("f");
      c.returnFromFunction();
    } c.endFunction();

    c.function("fib")
      .param("n", i8)
      .ret(i8)
      .begin();
    {
      c.declareLocal("f1", i8);
      c.declareLocal("f2", i8);
      
      // if (n <= 1) return n;
      c.jumpIf(c.le("n", literal::i8(1)), "done", "recurse");

      c.label("recurse");
      auto n_minus_1 = c.sub("n", literal::i8(1));
      auto n_minus_2 = c.sub("n", literal::i8(2));

      c.callFunction("fib").into("f1").arg(n_minus_1).done();
      c.callFunction("fib").into("f2").arg(n_minus_2).done();
	
      c.returnFromFunction(c.add("f1", "f2"));

      c.label("done");
      c.returnFromFunction("n");
      
    } c.endFunction();


  } c.endProgram();
 
  std::cout << c.brainfuck("fib") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
