#include <iostream>
#include "acus.h"

using namespace acus::api;

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Assembler c;

  c.program("test", "main").begin(); {


    c.declareGlobal("g", ts::i8());
  
    c.function("main").begin(); {
      c.declareLocal("x", ts::i8());
      c.assign("x", literal::i8('x'));
      c.referGlobals({"g"});
      c.assign("g", literal::i8('A'));
      c.callFunction("foo").done();
      c.writeOut("g");          // should print 'F'
      c.returnFromFunction();
    } c.endFunction();

    c.function("foo").begin(); {
      c.referGlobals({"g"});
      c.writeOut("g");          // should print 'A'
      c.assign("g", literal::i8('F'));  // modify global shadow
      c.returnFromFunction();
    } c.endFunction();
  


  } c.endProgram();

  std::cout << c.brainfuck("test") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
