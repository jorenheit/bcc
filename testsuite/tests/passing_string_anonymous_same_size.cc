// Pass the  string "Hello World" anonymously.
// Expected: "Hello World"

TEST_BEGIN

std::string str = "Hello World";
auto string = TypeSystem::string(str.size());

c.beginFunction("main"); {
  c.beginBlock("entry"); {
    c.callFunction("print", "return")(values::string(str));
  } c.endBlock();

  c.beginBlock("return"); {
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

auto printSig = TypeSystem::function(TypeSystem::voidT(), string);
c.beginFunction("print", printSig, {"s"}); {
  c.beginBlock("entry"); {
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

TEST_END
