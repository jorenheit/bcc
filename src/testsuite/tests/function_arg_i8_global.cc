// Tests passing a global i8 by value as a function argument, verifying that modifying the parameter does not mutate the global.
// Expected: GHG

TEST_BEGIN

c.declareGlobal("g", TypeSystem::i8());
    
c.beginFunction("main"); {
  c.referGlobals({"g"});
      
  c.beginBlock("entry"); {
    c.assign("g", values::i8('G'));
    auto args = c.constructFunctionArguments("g");
    c.callFunction("foo", "after_foo", args);
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.writeOut("g");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = c.constructFunctionSignature(TypeSystem::voidT(),
					   "arg1", TypeSystem::i8());

c.beginFunction("foo", fooSig); {
  c.beginBlock("entry"); {
    c.writeOut("arg1");
    c.assign("arg1", values::i8('H'));
    c.writeOut("arg1");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
