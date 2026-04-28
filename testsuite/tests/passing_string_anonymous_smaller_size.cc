// Store the string "Hello World" in a variable, then pass it to the print function that expects a bigger one
// Expected: "Hello World"


TEST_BEGIN

std::string str = "Hello World";
auto smallString = ts::string(str.size());
auto bigString = ts::string(str.size() * 2);
  
c.beginFunction("main"); {
  c.declareLocal("s", smallString);

  c.beginBlock("entry"); {
    c.assign("s", literal::string(str));
    c.callFunction("print", "return")("s");
  } c.endBlock();

  c.beginBlock("return"); {
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

auto printSig = ts::function(ts::voidT())(bigString);
c.beginFunction("print", printSig, {"s"}); {
  c.beginBlock("entry"); {
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

TEST_END
