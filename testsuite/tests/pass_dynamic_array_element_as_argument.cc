// Pass a runtime-selected array element as a function argument and print it inside the callee.
// Expected: "C"

TEST_BEGIN

auto array4 = TypeSystem::array(TypeSystem::i8(), 4);

c.beginFunction("main"); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", TypeSystem::i8());

  c.beginBlock("entry"); {
    c.assign("arr", values::array(TypeSystem::i8(),
				  values::i8('A'),
				  values::i8('B'),
				  values::i8('C'),
				  values::i8('D')));
    c.assign("idx", values::i8(2));

    auto elem = c.arrayElement("arr", "idx");
    auto args = c.constructFunctionArguments(elem);
    c.callFunction("printChar", "return", args);
  } c.endBlock();

  c.beginBlock("return"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto sig = TypeSystem::function(TypeSystem::voidT(), TypeSystem::i8());
c.beginFunction("printChar", sig, {"ch"}); {
  c.beginBlock("entry"); {
    c.writeOut("ch");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
