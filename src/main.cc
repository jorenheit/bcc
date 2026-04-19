#include <iostream>
#include "compiler.h"

#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main() try {
  Compiler c;
  c.setEntryPoint("main");

  c.begin(); {
    auto i8     = TypeSystem::i8();
    auto i16     = TypeSystem::i16();
    auto row = TypeSystem::array(i8, 2);
    auto mat = TypeSystem::array(row, 3);

    auto rowPtr = TypeSystem::pointer(row);
    auto i8Ptr = TypeSystem::pointer(i8);

    c.beginFunction("main"); {
      c.declareLocal("idx8", i8);
      c.declareLocal("idx16", i16);
      c.declareLocal("rowPtr0", rowPtr);
      c.declareLocal("rowPtr1", rowPtr);
      c.declareLocal("rowPtr2", rowPtr);
      c.declareLocal("valuePtr", i8Ptr);
      c.declareLocal("matrix", mat);

      c.beginBlock("entry"); {

	auto row0 = c.arrayElement("matrix", 0);
	auto row1 = c.arrayElement("matrix", 1);
	auto row2 = c.arrayElement("matrix", 2);

	c.assign(c.arrayElement(row0, 0), values::i8('A'));
	c.assign(c.arrayElement(row1, 0), values::i8('B'));
	c.assign(c.arrayElement(row2, 0), values::i8('C'));

	c.assign("rowPtr0", c.addressOf(row0));
	c.assign("rowPtr1", c.addressOf(row1));
	c.assign("rowPtr2", c.addressOf(row2));
	
	auto row0ref = c.dereferencePointer("rowPtr0");
	auto row1ref = c.dereferencePointer("rowPtr1");
	auto row2ref = c.dereferencePointer("rowPtr2");

	c.writeOut(c.arrayElement(row0ref, 0)); // A
	c.writeOut(c.arrayElement(row1ref, 0)); // B
	c.writeOut(c.arrayElement(row2ref, 0)); // C

	c.assign("idx8", values::i8(1));
	c.assign("idx16", values::i16(1));
	c.addAssign("rowPtr0", "idx16");

	c.writeOut(c.arrayElement(row0ref, 0)); // A
	c.writeOut(c.arrayElement(row1ref, 0)); // B
	c.writeOut(c.arrayElement(row2ref, 0)); // C
	
	// c.assign("q", c.add("p", "idx"));	
	// auto qArr = c.dereferencePointer("q");
	// c.writeOut(c.arrayElement(qArr, 0)); // C

	// c.assign("idx", values::i16(1));
	// c.subAssign("q", "idx");
	// auto qPrev = c.dereferencePointer("q");
	// c.writeOut(c.arrayElement(qPrev, 0)); // B

	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();


  } c.end();

  std::cout << c.dumpBrainfuck() << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
