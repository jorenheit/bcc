// Read and write through a pointer to a pointer
// Expected: AZZ

TEST_BEGIN

auto i8   = TypeSystem::i8();
auto i8p  = TypeSystem::pointer(i8);
auto i8pp = TypeSystem::pointer(i8p);

c.beginFunction("main"); {
  c.declareLocal("x", i8);
  c.declareLocal("p", i8p);
  c.declareLocal("pp", i8pp);

  c.beginBlock("entry"); {
    c.assign("x", values::i8('A'));
    c.assign("p", c.addressOf("x"));
    c.assign("pp", c.addressOf("p"));

    auto pFromPp = c.dereferencePointer("pp");
    auto xFromPp = c.dereferencePointer(pFromPp);

    c.writeOut(xFromPp);                 // A

    c.assign(xFromPp, values::i8('Z'));

    c.writeOut("x");                     // Z
    c.writeOut(c.dereferencePointer("p"));// Z

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
