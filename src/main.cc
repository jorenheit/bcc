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

  auto array = TypeSystem::array(TypeSystem::i8(), 4);
  
  c.begin(); {
    c.beginFunction("main"); {

      c.declareLocal("x", array);
      c.declareLocal("e", TypeSystem::i8());      
      c.declareLocal("i", TypeSystem::i8());
      
      c.beginBlock("entry"); {
	c.assign("x", values::array(TypeSystem::i8(), 'A', 'B', 'C', 'D'));
	c.assign("i", values::i8(2));
	c.arrayElement("x", "i", "e");	
	c.writeOut("e");
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
  } c.end();
  
  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
