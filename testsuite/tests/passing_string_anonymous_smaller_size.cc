// Store the string "Hello World" in a variable, then pass it to the print function that expects a bigger one
// Expected: "Hello World"


TEST_BEGIN

std::string str = "Hello World";
auto smallString = ts::string(str.size());
auto bigString = ts::string(str.size() * 2);
  
c.function("main").begin(); {
  c.declareLocal("s", smallString);

  c.block("entry").begin(); {
    c.assign("s", literal::string(str));
    c.callFunction("print", "return").arg("s").done();
  } c.endBlock();

  c.block("return").begin(); {
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

c.function("print").param("s", bigString).ret(ts::void_t()).begin(); {
  c.block("entry").begin(); {
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

TEST_END
