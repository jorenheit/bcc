// Tests an array of function pointers indexed with a dynamic i8 index.
// The same indirect call expression is used with index 0 and then index 1.
// Expected: AB

TEST_BEGIN

auto i8 = ts::i8();
auto voidT = ts::void_t();
auto fnType = ts::function().ret(voidT).done();
auto fnPtr = ts::function_pointer(fnType);
auto fnArray = ts::array(fnPtr, 2);

c.function("main").begin(); {
  c.declareLocal("arr", fnArray);
  c.declareLocal("idx", i8);

  c.assign(c.arrayElement("arr", 0), literal::function_pointer(fnType, "printA"));
  c.assign(c.arrayElement("arr", 1), literal::function_pointer(fnType, "printB"));
  c.assign("idx", literal::i8(0));
  c.callFunctionPointer(c.arrayElement("arr", "idx")).done();
  c.assign("idx", literal::i8(1));
  c.callFunctionPointer(c.arrayElement("arr", "idx")).done();
  c.returnFromFunction();
} c.endFunction();

c.function("printA").begin(); {
  c.write(literal::i8('A'));
  c.returnFromFunction();
} c.endFunction();

c.function("printB").begin(); {
  c.write(literal::i8('B'));
  c.returnFromFunction();
} c.endFunction();

TEST_END
