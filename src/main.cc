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
    auto array2 = TypeSystem::array(TypeSystem::i8(), 2);

    c.beginFunction("main"); {
      c.declareLocal("x", TypeSystem::i8());
      c.beginBlock("entry"); {
	c.assign("x", values::i8('B'));
	c.callFunction("foo", "after_foo", values::array(TypeSystem::i8(), 'A', "x"));
      } c.endBlock();

      c.beginBlock("after_foo"); {
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo", TypeSystem::voidT(),
		    "arr", array2); {
      c.beginBlock("entry"); {
	c.writeOut("arr");
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
