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

  auto i8  = TypeSystem::i8();
  auto i8p = TypeSystem::pointer(i8);

  c.begin(); {
    c.declareGlobal("g", i8);

    c.beginFunction("main"); {
      c.referGlobals({"g"});
      c.declareLocal("p", i8p);
      c.declareLocal("x", i8);

      c.beginBlock("entry"); {
	c.assign("g", values::i8('G'));
	c.assign("p", values::pointer(i8, "g"));

	auto pDeref = c.dereferencePointer("p");
	c.assign("x", pDeref);
	c.writeOut("g");
	c.writeOut("x");
	c.writeOut(pDeref);

	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
