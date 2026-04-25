// Pass "Hello World" anonymously to a function that expects a bigger one
// Expected: "Hello World"


TEST_BEGIN
std::string str = "Hello World";

c.beginFunction("main"); {
  c.beginBlock("entry"); {
    c.callFunction("print", "return")(values::string(str));
  } c.endBlock();

  c.beginBlock("return"); {
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

auto printSig = TypeSystem::function(TypeSystem::voidT(), TypeSystem::string(str.size() * 2));
c.beginFunction("print", printSig, {"s"}); {
  c.beginBlock("entry"); {
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

TEST_END
