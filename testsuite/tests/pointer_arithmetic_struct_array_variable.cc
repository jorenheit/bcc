// Do pointer arithmetic on an array of structs using a variable offset
// Expected: ACB

TEST_BEGIN

auto u8 = ts::u8();
auto pairT = ts::defineStruct("Pair").field("x", u8).field("y", u8).done();
auto pairP = ts::pointer(pairT);
auto arrT  = ts::array(pairT, 3);

c.function("main").begin(); {
  c.declareLocal("idx", ts::u16());
  c.declareLocal("p", pairP);
  c.declareLocal("q", pairP);
  c.declareLocal("arr", arrT);

  c.assign(c.structField(c.arrayElement("arr", 0), "x"), literal::u8('A'));
  c.assign(c.structField(c.arrayElement("arr", 1), "x"), literal::u8('B'));
  c.assign(c.structField(c.arrayElement("arr", 2), "x"), literal::u8('C'));

  c.assign("p", c.addressOf(c.arrayElement("arr", 0)));
  auto pObj = c.dereferencePointer("p");
  c.write(c.structField(pObj, "x")); // A

  c.assign("idx", literal::u16(2));
  c.assign("q", c.add("p", "idx"));
  auto qObj = c.dereferencePointer("q");
  c.write(c.structField(qObj, "x")); // C

  c.assign("idx", literal::u16(1));
  c.subAssign("q", "idx");
  auto qPrev = c.dereferencePointer("q");
  c.write(c.structField(qPrev, "x")); // B

  c.returnFromFunction();
} c.endFunction();

TEST_END
