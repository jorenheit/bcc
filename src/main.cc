#include <iostream>
#include "compiler.h"

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Compiler c;
  c.setEntryPoint("main");

  c.begin(); {

    auto i8 = TypeSystem::i8();
    auto i16 = TypeSystem::i16();

    c.beginFunction("main"); {
      c.declareLocal("a", i8);
      c.declareLocal("b", i8);
      c.declareLocal("x", i16);
      c.declareLocal("y", i16);

      c.beginBlock("entry"); {

	// 2. i8 variable + i8 variable: true XOR false -> 1
	c.assign("a", values::i8(1));
	c.assign("b", values::i8(1));
	// c.writeOut("a");
	// c.writeOut("b");
	c.writeOut(c.lxor("a", "b"));
	// c.writeOut(c.add(c.lxor("a", "b"), values::i8('A')));

	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
