// Tests an array of function pointers indexed dynamically inside a loop.
// The loop alternates idx between 0 and 1 and calls arr[idx] four times.
// Expected: ABAB

TEST_BEGIN

auto u8 = ts::u8();
auto voidT = ts::void_t();
auto actionType = ts::function().ret(voidT).done();
auto actionPtr = ts::function_pointer(actionType);
auto actionArray = ts::array(actionPtr, 2);

c.function("main").begin(); {
  c.declareLocal("arr", actionArray);
  c.declareLocal("idx", u8);
  c.declareLocal("count", u8);

  c.assign(c.arrayElement("arr", 0), literal::function_pointer(actionType, "printA"));
  c.assign(c.arrayElement("arr", 1), literal::function_pointer(actionType, "printB"));
  c.assign("idx", literal::u8(0));
  c.assign("count", literal::u8(0));
  c.label("check");
  c.jumpIf(c.lt("count", literal::u8(4)), "body", "end");
  c.label("body");
  c.callFunctionPointer(c.arrayElement("arr", "idx")).done();
  c.jumpIf("idx", "setZero", "setOne");
  c.label("setZero");
  c.assign("idx", literal::u8(0));
  c.assign("count", c.add("count", literal::u8(1)));
  c.jump("check");
  c.label("setOne");
  c.assign("idx", literal::u8(1));
  c.assign("count", c.add("count", literal::u8(1)));
  c.jump("check");
  c.label("end");
  c.returnFromFunction();
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
