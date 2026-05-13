// Convert i8 to i16
// Expected: "AA"

TEST_BEGIN

std::string str = "Hello World";

c.function("main").begin(); {
  c.declareLocal("x", ts::i8());
  c.declareLocal("y", ts::i16());

  c.assign("x", literal::i8('A'));
  c.write("x");
  c.assign("y", "x");
  c.write("y");
  c.returnFromFunction();
} c.endFunction();

TEST_END
