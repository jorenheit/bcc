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
  auto array2 = TypeSystem::array(array, 4);
  
  c.begin(); {
    c.beginFunction("main"); {

      c.declareLocal("x", array2);
      c.declareLocal("i", TypeSystem::i16());
      c.declareLocal("j", TypeSystem::i16());
      
      c.beginBlock("entry"); {
	c.assign("x", values::array(array,
				    values::array(TypeSystem::i8(), 'A', 'A', 'A', 'A'),
				    values::array(TypeSystem::i8(), 'B', 'B', 'B', 'B'),
				    values::array(TypeSystem::i8(), 'C', 'C', 'C', 'C'),
				    values::array(TypeSystem::i8(), 'D', 'D', 'D', 'D'))
		 );


	c.assign("i", values::i16(2));
	c.assign("j", values::i16(2));

	auto row = c.arrayElement("x", "i");
	auto elem = c.arrayElement(row, "j");

	c.assign(row, values::array(TypeSystem::i8(), 'X', 'X', 'X', 'X'));
	c.assign(elem, values::i8('Y'));
	c.writeOut("x");
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
  } c.end();
  
  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
