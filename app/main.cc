#include <iostream>
#include "acus.h"

using namespace acus::api;

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Assembler a;

  auto voidf = ts::void_function();
  auto fptr = ts::function_pointer(voidf);
  
  a.program("test", "main").begin(); {

    a.function("main").begin(); {
      a.declareLocal("x", ts::i8());
      a.declareLocal("f", fptr);

      a.assign("x", literal::i8(0));
      a.jumpIf("x", "true", "false");
      a.label("true");
      a.assign("f", literal::function_pointer(voidf, "foo"));
      a.jump("exec");
      a.label("false");
      a.assign("f", literal::function_pointer(voidf, "bar"));
      a.label("exec");
      a.callFunctionPointer("f").done();
      a.returnFromFunction();
    } a.endFunction();

    a.function("foo").begin(); {
      a.writeOut(literal::string("true"));
      a.returnFromFunction();
    } a.endFunction();
    
    a.function("bar").begin(); {
      a.writeOut(literal::string("false"));
      a.returnFromFunction();
    } a.endFunction();
    
  } a.endProgram();

  std::cout << a.brainfuck("test") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
