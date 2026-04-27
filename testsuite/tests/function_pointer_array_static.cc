// Tests an array of function pointers indexed with static indices.
// arr[1] is called first, then arr[0], so storage order and selected target are both checked.
// Expected: BA

TEST_BEGIN

auto voidT = TypeSystem::voidT();
auto fnType = TypeSystem::function(voidT);
auto fnPtr = TypeSystem::function_pointer(fnType);
auto fnArray = TypeSystem::array(fnPtr, 2);

c.beginFunction("main"); {
  c.declareLocal("arr", fnArray);

  c.beginBlock("entry"); {
    c.assign(c.arrayElement("arr", 0), values::function_pointer(fnType, "printA"));
    c.assign(c.arrayElement("arr", 1), values::function_pointer(fnType, "printB"));
    c.callFunctionPointer(c.arrayElement("arr", 1), "second")();
  } c.endBlock();

  c.beginBlock("second"); {
    c.callFunctionPointer(c.arrayElement("arr", 0), "end")();
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
