// Read and write through a pointer to a pointer
// Expected: AZZ

TEST_BEGIN

auto i8   = ts::i8();
auto i8p  = ts::pointer(i8);
auto i8pp = ts::pointer(i8p);

c.function("main").begin(); {
  c.declareLocal("x", i8);
  c.declareLocal("p", i8p);
  c.declareLocal("pp", i8pp);

  c.assign("x", literal::i8('A'));
  c.assign("p", c.addressOf("x"));
  c.assign("pp", c.addressOf("p"));

  auto pFromPp = c.dereferencePointer("pp");
  auto xFromPp = c.dereferencePointer(pFromPp);

  c.writeOut(xFromPp);                 // A

  c.assign(xFromPp, literal::i8('Z'));

  c.writeOut("x");                     // Z
  c.writeOut(c.dereferencePointer("p"));// Z

  c.returnFromFunction();
} c.endFunction();

TEST_END
