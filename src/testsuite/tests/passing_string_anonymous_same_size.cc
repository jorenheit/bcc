// Pass the  string "Hello World" anonymously.
// Expected: "Hello World"

TEST_BEGIN

std::string str = "Hello World";
auto string = TypeSystem::string(str.size());

c.beginFunction("main"); {
  c.beginBlock("entry"); {
    auto args = c.constructFunctionArguments(values::string(str));
    c.callFunction("print", "return", args);
  } c.endBlock();

  c.beginBlock("return"); {
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

auto printSig = c.constructFunctionSignature(TypeSystem::voidT(),
					     "s", string);
c.beginFunction("print", printSig); {
  c.beginBlock("entry"); {
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

TEST_END
