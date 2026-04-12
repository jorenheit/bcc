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
    c.beginFunction("main"); {

      c.declareLocal("x", i8);
      c.declareLocal("px", i8p);
   
      c.beginBlock("entry"); {
	c.assign("px", values::pointer(i8, "x"));	
	c.assign("x", values::i8('A'));
	c.callFunction("foo", "after1", "px");
      } c.endBlock();

      c.beginBlock("after1"); {
	c.assign("x", values::i8('B'));
	c.callFunction("foo", "after2", "px");	
      } c.endBlock();

      c.beginBlock("after2"); {
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
