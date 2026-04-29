#include <iostream>
#include "acus.h"

using namespace acus::api;

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Assembler c;

  c.program("name", "main").begin(); {
    
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

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
