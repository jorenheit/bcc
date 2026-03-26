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

  c.begin(); {
    c.beginFunction("main"); {
      c.declareLocal("s", point);

      c.beginBlock("entry"); {
	auto x = c.getStructField(values::ref("s"), "x");
	auto y = c.getStructField(values::ref("s"), "y");
      
	c.assign(x, values::i8('A'));
	c.assign(y, values::i8('B'));

	c.callFunction("bar", "next", x, y);
	
      } c.endBlock();

      c.beginBlock("next"); {
	c.callFunction("foo", "return", "s");
      } c.endBlock();
      
      c.beginBlock("return"); {
      	c.returnFromFunction();
      } c.endBlock();
      
      
    } c.endFunction();


    c.beginFunction("foo", TypeSystem::voidT(), "s", point); {
      c.beginBlock("entry"); {
	c.writeOut("s");
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("bar", TypeSystem::voidT(), "x", TypeSystem::i8(), "y", TypeSystem::i8()); {
      c.beginBlock("entry"); {
	c.writeOut("x");
	c.writeOut("y");
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
    
    
  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
