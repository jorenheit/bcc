// Tests an array of function pointers indexed with a dynamic i8 index.
// The same indirect call expression is used with index 0 and then index 1.
// Expected: AB

TEST_BEGIN

auto i8 = TypeSystem::i8();
auto voidT = TypeSystem::voidT();
auto fnType = TypeSystem::function(voidT);
auto fnPtr = TypeSystem::function_pointer(fnType);
auto fnArray = TypeSystem::array(fnPtr, 2);

c.beginFunction("main"); {
  c.declareLocal("arr", fnArray);
  c.declareLocal("idx", i8);

  c.beginBlock("entry"); {
    c.assign(c.arrayElement("arr", 0), values::function_pointer(fnType, "printA"));
    c.assign(c.arrayElement("arr", 1), values::function_pointer(fnType, "printB"));
    c.assign("idx", values::i8(0));
    c.callFunctionPointer(c.arrayElement("arr", "idx"), "second")();
  } c.endBlock();

  c.beginBlock("second"); {
    c.assign("idx", values::i8(1));
    c.callFunctionPointer(c.arrayElement("arr", "idx"), "end")();
  } c.endBlock();

  c.beginBlock("end"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("printA"); {
  c.beginBlock("entry"); {
    c.writeOut(values::i8('A'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("printB"); {
  c.beginBlock("entry"); {
    c.writeOut(values::i8('B'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
