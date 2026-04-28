#include <iostream>
#include "acus.h"

using namespace acus::api;

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Builder c;
  c.setEntryPoint("main");

  c.begin(); {

    
    auto i8 = ts::i8();
    auto i16 = ts::i16();
    auto voidT = ts::voidT();
    auto fooType = ts::function(voidT)();
    auto fooPtr = ts::function_pointer(fooType);

    std::vector<types::NameTypePair> fields;
    fields.emplace_back("x", i8);
    fields.emplace_back("y", i8);
    auto point = ts::defineStruct("Point")(fields);
    auto val = literal::structT(point)(literal::i8(1), literal::i8(2));
    
    
    c.beginFunction("main"); {
      c.declareLocal("fptr", fooPtr);
      
      c.beginBlock("entry"); {
	c.callFunction("getPtr", "next", "fptr")(literal::i8(0));
      } c.endBlock();

      c.beginBlock("next"); {
	c.callFunctionPointer("fptr", "end")();
      } c.endBlock();

      c.beginBlock("end"); {
	c.returnFromFunction();
      } c.endBlock();
      
    } c.endFunction();


    auto getPtrType = ts::function(fooPtr)(i8);
    c.beginFunction("getPtr", getPtrType, {"x"}); {

      c.beginBlock("entry"); {
	c.branchIf("x", "true", "false");
      } c.endBlock();

      c.beginBlock("true"); {
	c.returnFromFunction(literal::function_pointer(fooType, "foo1"));
      } c.endBlock();

      c.beginBlock("false"); {
	c.returnFromFunction(literal::function_pointer(fooType, "foo2"));
      } c.endBlock();

    } c.endFunction();

    c.beginFunction("foo1"); {
      c.beginBlock("entry"); {
	c.writeOut(literal::string("foo1"));
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo2"); {
      c.beginBlock("entry"); {
	c.writeOut(literal::string("foo2"));
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
