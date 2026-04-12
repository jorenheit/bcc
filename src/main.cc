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

  types::TypeHandle i8 = TypeSystem::i8();
  types::TypeHandle i8p = TypeSystem::pointer(TypeSystem::i8());
  
  c.setEntryPoint("main");

  c.begin(); {
    c.declareGlobal("g", i8);
    
    c.beginFunction("main"); {
      c.referGlobals({"g"});
      c.declareLocal("pg", i8p);
      c.declareLocal("x", i8);
   
      c.beginBlock("entry"); {
	c.assign("pg", values::pointer(i8, "g"));
	c.assign("g", values::i8('G'));
	c.assign("x", values::i8('X'));
	c.callFunction("foo", "after", "pg");
      } c.endBlock();

      c.beginBlock("after"); {
	c.writeOut("x");
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo", TypeSystem::voidT(), "p", i8p); {
      c.beginBlock("entry"); {
	auto pDeref = c.dereferencePointer("p");
	c.writeOut(pDeref);
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
    
  } c.end();
  
  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
