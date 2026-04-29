// Pass "Hello World" anonymously to a function that expects a bigger one
// Expected: "Hello World"


TEST_BEGIN
std::string str = "Hello World";

c.function("main").begin(); {
  c.block("entry").begin(); {
    c.callFunction("print", "return").arg(literal::string(str)).done();
  } c.endBlock();

  c.block("return").begin(); {
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

c.function("print").param("s", ts::string(str.size() * 2)).ret(ts::void_t()).begin(); {
  c.block("entry").begin(); {
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

TEST_END
