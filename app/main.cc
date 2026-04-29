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
    auto voidT = ts::void_t();
    auto array3 = ts::array(i8, 3);

    auto point = ts::defineStruct("Point")
      .field("x", i8)
      .field("y", i8)
      .done();

    auto x = literal::array(array3)
      .push(literal::i8('A'))
      .push(literal::i8('B'))
      .push(literal::i8('C'))
      .done();
      
    auto val = literal::struct_t(point)
      .init("x", literal::i8(1))
      .init("y", literal::i8(2))
      .done();
    
    auto fooType = ts::function().ret(point).done();
    auto fooPtr = ts::function_pointer(fooType);

    c.beginFunction("main"); {
      c.declareLocal("x", point);
      c.declareLocal("fooPtr", fooPtr);
      
      c.beginBlock("entry"); {
	
	c.assign("fooPtr", literal::function_pointer(fooType, "foo"));
	c.callFunctionPointer("fooPtr", "end").into("x").done();
      } c.endBlock();

      c.beginBlock("end"); {
	c.writeOut("x");
	c.returnFromFunction();
      } c.endBlock();
      
    } c.endFunction();

    c.beginFunction("foo", fooType); {
      c.beginBlock("entry"); {

	auto val = literal::struct_t(point)
	  .init("x", literal::i8('X'))
	  .init("y", literal::i8('Y'))
	  .done();
	
	c.returnFromFunction(val);
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("bar", fooType); {
      c.beginBlock("entry"); {

	auto val = literal::struct_t(point)
	  .init("x", literal::i8('Q'))
	  .init("y", literal::i8('W'))
	  .done();
	
	c.returnFromFunction(val);
      } c.endBlock();
    } c.endFunction();

  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
