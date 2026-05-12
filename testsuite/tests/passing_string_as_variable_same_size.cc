// Store the string "Hello World" in a variable, then pass it to the print function
// Expected: "Hello World"


TEST_BEGIN

std::string str = "Hello World";
auto string = ts::string(str.size());

c.function("main").begin(); {
  c.declareLocal("s", string);

  c.assign("s", literal::string(str));
  c.callFunction("print").arg("s").done();
  c.returnFromFunction();
} c.endFunction();

c.function("print").param("s", string).ret(ts::void_t()).begin(); {
  c.print("s");
  c.returnFromFunction();
} c.endFunction();

TEST_END
