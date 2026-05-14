// Tests a function pointer call whose callee returns another function pointer.
// The returned pointer is then called, then the selector pointer is overwritten
// and the sequence is repeated. This stresses indirect call return transport
// and overwriting pointer locals after indirect calls.
// Expected: SATB

TEST_BEGIN

auto voidT = ts::void_t();
auto actionType = ts::function().ret(voidT).done();
auto actionPtr = ts::function_pointer(actionType);
auto selectorType = ts::function().ret(actionPtr).done();
auto selectorPtr = ts::function_pointer(selectorType);

c.function("main").begin(); {
  c.declareLocal("selector", selectorPtr);
  c.declareLocal("action", actionPtr);

  c.assign("selector", literal::function_pointer(selectorType, "selectA"));
  c.callFunctionPointer("selector").into("action").done();
  c.callFunctionPointer("action").done();
  c.assign("selector", literal::function_pointer(selectorType, "selectB"));
  c.callFunctionPointer("selector").into("action").done();
  c.callFunctionPointer("action").done();
  c.returnFromFunction();
} c.endFunction();

c.function("selectA").ret(actionPtr).begin(); {
  c.write(literal::u8('S'));
  c.returnFromFunction(literal::function_pointer(actionType, "printA"));
} c.endFunction();

c.function("selectB").ret(actionPtr).begin(); {
  c.write(literal::u8('T'));
  c.returnFromFunction(literal::function_pointer(actionType, "printB"));
} c.endFunction();

c.function("printA").begin(); {
  c.write(literal::u8('A'));
  c.returnFromFunction();
} c.endFunction();

c.function("printB").begin(); {
  c.write(literal::u8('B'));
  c.returnFromFunction();
} c.endFunction();

TEST_END
