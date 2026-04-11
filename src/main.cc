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

  auto point = c.defineStruct("Point",
			      "x", TypeSystem::i8(),
			      "y", TypeSystem::i8());
  
  auto array = TypeSystem::array(point, 4);
  auto array2 = TypeSystem::array(array, 4);
  
  c.begin(); {
    c.beginFunction("main"); {

      c.declareLocal("x", array2);
      c.declareLocal("i", TypeSystem::i16());
      c.declareLocal("j", TypeSystem::i16());
      
      c.beginBlock("entry"); {
	c.assign("i", values::i16(2));
	c.assign("j", values::i16(2));

	auto row = c.arrayElement("x", "i");
	auto elem = c.arrayElement(row, "j");
	auto field = c.structField(elem, "y");
	
	c.assign(row, values::array(point,
				    values::structT(point, values::i8(1), values::i8(2)),
				    values::structT(point, values::i8(3), values::i8(4)),
				    values::structT(point, values::i8(5), values::i8(6)),
				    values::structT(point, values::i8(7), values::i8(8)))
		 );

	c.writeOut(field);
	
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
  } c.end();
  
  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
