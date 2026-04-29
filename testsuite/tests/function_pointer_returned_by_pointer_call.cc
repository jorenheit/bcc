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

  c.block("entry").begin(); {
    c.assign("selector", literal::function_pointer(selectorType, "selectA"));
    c.callFunctionPointer("selector", "callA").into("action").done();
  } c.endBlock();

  c.block("callA").begin(); {
    c.callFunctionPointer("action", "selectB").done();
  } c.endBlock();

  c.block("selectB").begin(); {
    c.assign("selector", literal::function_pointer(selectorType, "selectB"));
    c.callFunctionPointer("selector", "callB").into("action").done();
  } c.endBlock();

  c.block("callB").begin(); {
    c.callFunctionPointer("action", "end").done();
  } c.endBlock();

  c.block("end").begin(); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("selectA").ret(actionPtr).begin(); {
  c.block("entry").begin(); {
    c.writeOut(literal::i8('S'));
    c.returnFromFunction(literal::function_pointer(actionType, "printA"));
  } c.endBlock();
} c.endFunction();

c.function("selectB").ret(actionPtr).begin(); {
  c.block("entry").begin(); {
    c.writeOut(literal::i8('T'));
    c.returnFromFunction(literal::function_pointer(actionType, "printB"));
  } c.endBlock();
} c.endFunction();

c.function("printA").begin(); {
  c.block("entry").begin(); {
    c.writeOut(literal::i8('A'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("printB").begin(); {
  c.block("entry").begin(); {
    c.writeOut(literal::i8('B'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
