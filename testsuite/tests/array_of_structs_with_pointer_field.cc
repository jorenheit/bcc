// Tests recursive pointer relocation through array -> struct -> pointer.
// foo reads through both pointer fields, then writes through both.
// Expected: ABXY

TEST_BEGIN

auto i8  = ts::i8();
auto i8p = ts::pointer(i8);

auto holder = ts::defineStruct("Holder").field("p", i8p).done();
auto holders = ts::array(holder, 2);

c.function("main").begin(); {
  c.declareLocal("arr", holders);
  c.declareLocal("a", i8);
  c.declareLocal("b", i8);

  c.assign("a", literal::i8('A'));
  c.assign("b", literal::i8('B'));

  c.assign(c.structField(c.arrayElement("arr", 0), "p"), c.addressOf("a"));
  c.assign(c.structField(c.arrayElement("arr", 1), "p"), c.addressOf("b"));

  c.callFunction("foo").arg("arr").done();
  c.write("a");
  c.write("b");
  c.returnFromFunction();
} c.endFunction();


c.function("foo").param("arr", holders).ret(ts::void_t()).begin(); {
  auto p0 = c.structField(c.arrayElement("arr", 0), "p");
  auto p1 = c.structField(c.arrayElement("arr", 1), "p");

  auto d0 = c.dereferencePointer(p0);
  auto d1 = c.dereferencePointer(p1);

  c.write(d0);
  c.write(d1);

  c.assign(d0, literal::i8('X'));
  c.assign(d1, literal::i8('Y'));

  c.returnFromFunction();
} c.endFunction();

TEST_END
