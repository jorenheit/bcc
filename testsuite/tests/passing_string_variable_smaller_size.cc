// Pass "Hello World" anonymously to a function that expects a bigger one
// Expected: "Hello World"


TEST_BEGIN
std::string str = "Hello World";

c.function("main").begin(); {
  c.callFunction("print").arg(literal::string(str)).done();
  c.returnFromFunction();
} c.endFunction();

c.function("print").param("s", ts::string(str.size() * 2)).ret(ts::void_t()).begin(); {
  c.writeOut("s");
  c.returnFromFunction();
} c.endFunction();

TEST_END
