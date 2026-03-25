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
  auto &ts = c.typeSystem();

  c.setEntryPoint("main");
  
  // TODO: simplify syntax
  auto point = c.defineStruct("Point",
			      types::StructType::Field{"x", ts.i8()},
			      types::StructType::Field{"y", ts.i8()});

  
  c.begin(); {
    c.beginFunction("main"); {
      c.declareLocal("s", point);

      
      c.beginBlock("entry"); {
	auto x = c.getStructField(values::var("s"), "x");
	auto y = c.getStructField(values::var("s"), "y");
      
	c.assign(x, values::value(ts.i8(), 'A'));
	c.assign(y, values::value(ts.i8(), 'B'));

	std::vector<Slot> args = c.constructArgList(x, y);
	
	c.callFunction("bar", "return", args);
      } c.endBlock();
      
      c.beginBlock("return"); {
      	c.returnFromFunction();
      } c.endBlock();
      
    } c.endFunction();

    c.beginFunction("foo", ts.voidT(), "s", point); {
      c.writeOut("s");
      c.returnFromFunction();
    } c.endFunction();

    c.beginFunction("bar", ts.voidT(), "x", ts.i8(), "y", ts.i8()); {
      c.writeOut("x");
      c.writeOut("y");
      c.returnFromFunction();
    } c.endFunction();
    
    
  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
