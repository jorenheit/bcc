
TEST_BEGIN
types::TypeHandle array2 = TypeSystem::array(TypeSystem::i8(), 2);
types::TypeHandle array22 = TypeSystem::array(array2, 2);
  
c.beginFunction("main"); {
  c.declareLocal("B1", TypeSystem::i8());
  c.declareLocal("B2", TypeSystem::i8());
  c.declareLocal("E", TypeSystem::i8());

  c.beginBlock("entry"); {
    auto B = values::i8('B');
    auto C = values::i8('C');
    c.assign("B1", B);
    c.assign("B2", "B1");
    auto arr0 = values::array(TypeSystem::i8(), 'A', "B2");
    auto arr1 = values::array(TypeSystem::i8(), C, 'D');
    auto arrarr = values::array(array2, arr0, arr1);

    auto args = c.constructFunctionArguments(arrarr);
    c.callFunction("foo", "after_foo", args, "E");
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.writeOut("E");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = c.constructFunctionSignature(TypeSystem::i8(), "arr", array22);
c.beginFunction("foo", fooSig); {
  c.declareLocal("E", TypeSystem::i8());
  c.beginBlock("entry"); {
    c.writeOut("arr");
    auto E = values::i8('E');
    c.assign("E", E);
	
    c.returnFromFunction("E");
  } c.endBlock();
} c.endFunction();

TEST_END
