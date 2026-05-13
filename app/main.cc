#include <iostream>
#include "acus.h"

using namespace acus::api;

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Assembler c;

  c.program("test", "main").begin(); {

    auto i8    = ts::i8();
    auto pairT = ts::defineStruct("Pair").field("x", i8).field("y", i8).done();
    auto pairP = ts::pointer(pairT);
    auto arrT  = ts::array(pairT, 3);

    c.function("main").begin(); {
      c.declareLocal("p", pairP);
      c.declareLocal("q", pairP);
      c.declareLocal("arr", arrT);

      c.assign(c.structField(c.arrayElement("arr", 0), "x"), literal::i8('A'));
      c.assign(c.structField(c.arrayElement("arr", 1), "x"), literal::i8('B'));
      c.assign(c.structField(c.arrayElement("arr", 2), "x"), literal::i8('C'));

      c.assign("p", c.addressOf(c.arrayElement("arr", 0)));

      auto pObj = c.dereferencePointer("p");
      c.write(c.structField(pObj, "x"));        // A

      // c.assign("q", c.add("p", literal::i16(2)));
      // auto qObj = c.dereferencePointer("q");
      // c.write(c.structField(qObj, "x"));        // C

      // c.subAssign("q", literal::i16(1));
      // auto qPrev = c.dereferencePointer("q");
      // c.write(c.structField(qPrev, "x"));       // B

      c.returnFromFunction();
    } c.endFunction();

  } c.endProgram();

  std::cout << c.brainfuck("test") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
