// Tests an array of function pointers indexed dynamically inside a loop.
// The loop alternates idx between 0 and 1 and calls arr[idx] four times.
// Expected: ABAB

TEST_BEGIN

auto i8 = ts::i8();
auto voidT = ts::void_t();
auto actionType = ts::function().ret(voidT).done();
auto actionPtr = ts::function_pointer(actionType);
auto actionArray = ts::array(actionPtr, 2);

c.function("main").begin(); {
  c.declareLocal("arr", actionArray);
  c.declareLocal("idx", i8);
  c.declareLocal("count", i8);

  c.block("entry").begin(); {
    c.assign(c.arrayElement("arr", 0), literal::function_pointer(actionType, "printA"));
    c.assign(c.arrayElement("arr", 1), literal::function_pointer(actionType, "printB"));
    c.assign("idx", literal::i8(0));
    c.assign("count", literal::i8(0));
    c.setNextBlock("check");
  } c.endBlock();

  c.block("check").begin(); {
    c.branchIf(c.lt("count", literal::i8(4)), "body", "end");
  } c.endBlock();

  c.block("body").begin(); {
    c.callFunctionPointer(c.arrayElement("arr", "idx"), "toggle").done();
  } c.endBlock();

  c.block("toggle").begin(); {
    c.branchIf("idx", "setZero", "setOne");
  } c.endBlock();

  c.block("setZero").begin(); {
    c.assign("idx", literal::i8(0));
    c.assign("count", c.add("count", literal::i8(1)));
    c.setNextBlock("check");
  } c.endBlock();

  c.block("setOne").begin(); {
    c.assign("idx", literal::i8(1));
    c.assign("count", c.add("count", literal::i8(1)));
    c.setNextBlock("check");
  } c.endBlock();

  c.block("end").begin(); {
    c.returnFromFunction();
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
