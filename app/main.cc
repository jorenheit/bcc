#include <iostream>
#include "acus.h"

using namespace acus::api;

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Assembler c;

  c.program("test", "main").begin(); {

    auto u8    = ts::u8();
    auto pairT = ts::defineStruct("Pair").field("x", u8).field("y", u8).done();
    auto pairP = ts::pointer(pairT);
    auto arrT  = ts::array(pairT, 3);
    auto string = ts::string(10);
    
    c.function("main").begin(); {
      c.declareLocal("str", string);
      c.declareLocal("idx", u8);
      c.assign("str", literal::string("Hello"));
      
      Literal u8_1 = literal::u8(1);
      c.assign("idx", u8_1);
      c.print("str");
      c.write(c.arrayElement("str", u8_1));
      //c.write(c.arrayElement("str", "idx"));
      
      c.returnFromFunction();
    } c.endFunction();

  } c.endProgram();

  std::cout << c.brainfuck("test") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
