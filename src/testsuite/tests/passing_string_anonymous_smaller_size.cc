// Store the string "Hello World" in a variable, then pass it to the print function that expects a bigger one
// Expected: "Hello World"


TEST_BEGIN

std::string str = "Hello World";
auto smallString = TypeSystem::string(str.size());
auto bigString = TypeSystem::string(str.size() * 2);
  
c.beginFunction("main"); {
  c.declareLocal("s", smallString);

  c.beginBlock("entry"); {
    c.assign("s", values::string(str));
    auto args = c.constructFunctionArguments("s");
    c.callFunction("print", "return", args);
  } c.endBlock();

  c.beginBlock("return"); {
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

auto printSig = c.constructFunctionSignature(TypeSystem::voidT(),
					     "s", bigString);
c.beginFunction("print", printSig); {
  c.beginBlock("entry"); {
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

TEST_END
