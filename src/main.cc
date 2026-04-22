#include <iostream>
#include "compiler.h"

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Compiler c;
  c.setEntryPoint("main");

  c.begin(); {
    // compare variable rhs vs literal rhs for the same modulo
    auto i16 = TypeSystem::i16();

    c.beginFunction("main"); {
      c.declareLocal("x", i16);
      c.declareLocal("y", i16);

      c.beginBlock("entry"); {
	// c.assign("x", values::i16(0x7171));
	// c.assign("y", values::i16(0x5050));
	// c.writeOut(c.add(c.mod("x", "y"), values::i16(0x2424)));              // should be EE

	c.assign("x", values::i16(0x7171));
	// c.assign("y", values::i16(0x5050));
	//	c.writeOut("x");
	//	c.writeOut("y");
	c.writeOut(c.mod("x", values::i16(0x5050)));
	// c.writeOut(c.mod("x", "y"));
	
	//	c.writeOut(c.add(c.mod("x", values::i16(0x5050)), values::i16(0x2424))); // should also be EE

	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();    
  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
