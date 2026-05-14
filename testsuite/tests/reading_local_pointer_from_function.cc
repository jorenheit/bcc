// Tests reading through a pointer that points to a caller local and is passed to a function.
// Expected: XX

TEST_BEGIN

auto u8  = ts::u8();
auto u8p = ts::pointer(u8);

c.function("main").begin(); {
  c.declareLocal("p", u8p);
  c.declareLocal("x", u8);

  c.assign("x", literal::u8('X'));
  c.assign("p", c.addressOf("x")); //literal::pointer(u8, "x"));
  c.callFunction("foo").arg("p").done();
  c.write("x");
  c.returnFromFunction();
} c.endFunction();

c.function("foo").param("p", u8p).ret(ts::void_t()).begin(); {
  auto pDeref = c.dereferencePointer("p");
  c.write(pDeref);
  c.returnFromFunction();
} c.endFunction();

TEST_END
