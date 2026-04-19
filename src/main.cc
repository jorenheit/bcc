#include <iostream>
#include "compiler.h"

// TODO: add struct tests
// 1. writing and reading from fields
// 2. passing fields to functions
// 3. global struct
// 4. writing return value to field
// 5. passing entire structs to functions
// 6. returning structs from functions
#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Compiler c;
  c.setEntryPoint("main");
  c.begin(); {

    
    auto i8  = TypeSystem::i8();
    auto arr = TypeSystem::array(i8, 3);

    c.beginFunction("main"); {
      c.declareLocal("arr", arr);

      c.beginBlock("entry"); {
	c.assign("arr", values::array(i8,
				      values::i8('A'),
				      values::i8('B'),
				      values::i16('C')));
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
