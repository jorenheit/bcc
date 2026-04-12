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


  auto i8   = TypeSystem::i8();
  auto i8p  = TypeSystem::pointer(i8);
  auto i8pa = TypeSystem::array(i8p, 2);

  c.begin(); {
    c.beginFunction("main"); {
      c.declareLocal("p", i8pa);
      c.declareLocal("a", i8);
      c.declareLocal("b", i8);

      c.beginBlock("entry"); {
	c.assign("a", values::i8('A'));
	c.assign("b", values::i8('B'));

	c.assign(c.arrayElement("p", 0), values::pointer(i8, "a"));
	c.assign(c.arrayElement("p", 1), values::pointer(i8, "b"));

	c.callFunction("foo", "after", "p");
      } c.endBlock();

      c.beginBlock("after"); {
	c.writeOut("a");
	c.writeOut("b");
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo", TypeSystem::voidT(), "p", i8pa); {
      c.beginBlock("entry"); {
	auto p0Deref = c.dereferencePointer(c.arrayElement("p", 0));
	auto p1Deref = c.dereferencePointer(c.arrayElement("p", 1));

	c.writeOut(p0Deref);
	c.writeOut(p1Deref);

	c.assign(p0Deref, values::i8('X'));
	c.assign(p1Deref, values::i8('Y'));

	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
