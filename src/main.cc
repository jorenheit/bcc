#include <iostream>
#include "compiler.h"

int main() {
  Compiler c;
  auto &ts = c.typeSystem();

  
  c.setEntryPoint("main");

  using namespace types;
  c.begin(); {
    c.beginFunction("main"); {
      c.beginBlock("entry"); {
	c.writeOut(values::value(ts.i8(), 'A'));
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
}
