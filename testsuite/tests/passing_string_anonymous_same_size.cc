// Pass the  string "Hello World" anonymously.
// Expected: "Hello World"

TEST_BEGIN

std::string str = "Hello World";
auto string = ts::string(str.size());

c.beginFunction("main"); {
  c.beginBlock("entry"); {
    c.callFunction("print", "return")(literal::string(str));
  } c.endBlock();

  c.beginBlock("return"); {
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

auto printSig = ts::function(ts::voidT())(string);
c.beginFunction("print", printSig, {"s"}); {
  c.beginBlock("entry"); {
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

TEST_END
