// Pass "Hello World" anonymously to a function that expects a bigger one
// Expected: "Hello World"


TEST_BEGIN
std::string str = "Hello World";

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
					     "s", TypeSystem::string(str.size() * 2));
c.beginFunction("print", printSig); {
  c.beginBlock("entry"); {
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

TEST_END
