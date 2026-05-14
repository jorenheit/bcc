#include <iostream>
#include "acus/assembler/assembler.h"
using namespace acus;


int main() try {
  Assembler c;

  c.program("fib", "main").begin(); {

    auto u8 = ts::u8();
					    
    c.function("main").begin(); {
      c.declareLocal("f", u8);
      c.callFunction("fib").into("f").arg(literal::u8(12)).done();
      c.print("f");
      c.returnFromFunction();
    } c.endFunction();

    c.function("fib")
      .param("n", u8)
      .ret(u8)
      .begin();
    {
      c.declareLocal("f1", u8);
      c.declareLocal("f2", u8);
      
      // if (n <= 1) return n;
      c.jumpIf(c.le("n", literal::u8(1)), "done", "recurse");

      c.label("recurse");
      auto n_minus_1 = c.sub("n", literal::u8(1));
      auto n_minus_2 = c.sub("n", literal::u8(2));

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
