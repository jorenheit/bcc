#include <iostream>
#include "compiler.h"

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Compiler c;
  c.setEntryPoint("main");

  c.begin(); {

    c.beginFunction("main"); {
      c.declareLocal("x", TypeSystem::i16());
      c.declareLocal("y", TypeSystem::i8());
      c.declareLocal("z", TypeSystem::i16());

      c.beginBlock("entry"); {
	c.assign("x", values::i16(100));
	c.assign("y", values::i8(34));

	c.modAssign("x", "y"); // 100 % 34 = 0x0020
	// c.writeOut("x");
	// c.writeOut(values::i16(CAT('A', '!')));
	c.writeOut(c.add("x", values::i16(CAT('!', 'A')))); // 0x0020 + 0x4121 = 0x4141 -> AA

	c.assign("x", values::i16(101));
	c.assign("z", c.add(c.mod("x", "y"), values::i16(CAT('!', 'B')))); // 0x0021 + 0x4221 = 0x4242 -> BB
	c.writeOut("z"); // BB

	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
    
  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
