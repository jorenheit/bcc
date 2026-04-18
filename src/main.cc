#include <iostream>
#include "compiler.h"

// TODO: add struct tests
// 1. writing and reading from fields
// 2. passing fields to functions
// 3. global struct
// 4. writing return value to field
// 5. passing entire structs to functions
// 6. returning structs from functions

int main() try {
  Compiler c;
  c.setEntryPoint("main");
  c.begin(); {

    c.beginFunction("main"); {
      c.declareLocal("x", TypeSystem::i16());
      c.declareLocal("y", TypeSystem::i16());
      c.declareLocal("z", TypeSystem::i16());

      c.beginBlock("entry"); {
	c.assign("x", values::i16(257));
	c.assign("y", values::i8(1));

	c.writeOut("x");
	c.subAssign("x", "y");
	c.writeOut("x");
	c.subAssign("x", "y");
	c.writeOut("x");
	c.subAssign("x", "y");
	c.writeOut("x");
	c.subAssign("x", "y");
	c.writeOut("x");
	c.subAssign("x", "y");

	
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
