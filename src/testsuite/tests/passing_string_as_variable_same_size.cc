// Store the string "Hello World" in a variable, then pass it to the print function
// Expected: "Hello World"


TEST_BEGIN

std::string str = "Hello World";
auto string = TypeSystem::string(str.size());

c.beginFunction("main"); {
  c.declareLocal("s", string);

  c.beginBlock("entry"); {
    c.assign("s", values::string(str));
    c.callFunction("print", "return", "s");
  } c.endBlock();

  c.beginBlock("return"); {
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

c.beginFunction("print", TypeSystem::voidT(),
		"s", string); {
  c.beginBlock("entry"); {
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

TEST_END
