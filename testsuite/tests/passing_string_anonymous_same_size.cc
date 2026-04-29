// Pass the  string "Hello World" anonymously.
// Expected: "Hello World"

TEST_BEGIN

std::string str = "Hello World";
auto string = ts::string(str.size());

c.function("main").begin(); {
  c.block("entry").begin(); {
    c.callFunction("print", "return").arg(literal::string(str)).done();
  } c.endBlock();

  c.block("return").begin(); {
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

c.function("print").param("s", string).ret(ts::void_t()).begin(); {
  c.block("entry").begin(); {
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
      
} c.endFunction();

TEST_END
