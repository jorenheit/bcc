#include <iostream>
#include "acus/builder/builder.h"
using namespace acus;

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Builder c;
  c.setEntryPoint("main");

  c.begin(); {

    auto i8 = TypeSystem::i8();
    auto voidT = TypeSystem::voidT();
    auto fooType = TypeSystem::function(voidT);
    auto fooPtr = TypeSystem::function_pointer(fooType);
					    
    c.beginFunction("main"); {
      c.declareLocal("fptr", fooPtr);
      
      c.beginBlock("entry"); {
	c.callFunction("getPtr", "next", "fptr")(values::i8(0));
      } c.endBlock();

      c.beginBlock("next"); {
	c.callFunctionPointer("fptr", "end")();
      } c.endBlock();

      c.beginBlock("end"); {
	c.returnFromFunction();
      } c.endBlock();
      
    } c.endFunction();


    auto getPtrType = TypeSystem::function(fooPtr, {i8});
    c.beginFunction("getPtr", getPtrType, {"x"}); {

      c.beginBlock("entry"); {
	c.branchIf("x", "true", "false");
      } c.endBlock();

      c.beginBlock("true"); {
	c.returnFromFunction(values::function_pointer(fooType, "foo1"));
      } c.endBlock();

      c.beginBlock("false"); {
	c.returnFromFunction(values::function_pointer(fooType, "foo2"));
      } c.endBlock();

    } c.endFunction();

    c.beginFunction("foo1"); {
      c.beginBlock("entry"); {
	c.writeOut(values::string("foo1"));
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo2"); {
      c.beginBlock("entry"); {
	c.writeOut(values::string("foo2"));
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
