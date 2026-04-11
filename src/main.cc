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
auto point = c.defineStruct("Point",
			    "x", TypeSystem::i8(),
			    "y", TypeSystem::i8());

 c.declareGlobal("g", point);

 c.beginFunction("main"); {
   c.referGlobals({"g"});
      
   c.beginBlock("entry"); {
     auto gx = c.structField("g", "x");
     auto gy = c.structField("g", "y");

     c.assign(gx, values::i8('A'));
     c.assign(gy, values::i8('B'));
     c.writeOut("g");

     c.callFunction("foo", "after_foo");
   } c.endBlock();

   c.beginBlock("after_foo"); {
     c.writeOut("g");
     c.returnFromFunction();
   } c.endBlock();
 } c.endFunction();

 c.beginFunction("foo"); {
   c.referGlobals({"g"});
      
   c.beginBlock("entry"); {
     auto gy = c.structField("g", "y");
     c.assign(gy, values::i8('C'));
     c.returnFromFunction();
   } c.endBlock();
 } c.endFunction();
  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
