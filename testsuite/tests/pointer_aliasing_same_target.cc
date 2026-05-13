// Two pointers alias the same target and observe the same write
// Expected: XX

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);

c.function("main").begin(); {
  c.declareLocal("x", i8);
  c.declareLocal("p", i8p);
  c.declareLocal("q", i8p);

  c.assign("x", literal::i8('A'));
  c.assign("p", c.addressOf("x"));
  c.assign("q", c.addressOf("x"));

  c.assign(c.dereferencePointer("p"), literal::i8('X'));

  c.write(c.dereferencePointer("p")); // X
  c.write(c.dereferencePointer("q")); // X

  c.returnFromFunction();
} c.endFunction();

TEST_END
