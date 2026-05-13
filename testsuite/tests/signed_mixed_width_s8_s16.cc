// Signed mixed-width arithmetic: s8 with s16 promotes to s16
// Expected: ABAB

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("x", ts::s8());
  c.declareLocal("y", ts::s16());
  c.declareLocal("z", ts::s16());

  // s8(-1) + s16(0x4242) -> s16(0x4241) -> AB
  c.assign("x", literal::s8(-1));
  c.assign("y", literal::s16(CAT('B', 'B')));
  c.assign("z", c.add("x", "y"));
  c.write("z"); // AB

  // s16(0x4242) - s8(1) -> s16(0x4241) -> AB
  c.assign("x", literal::s8(1));
  c.assign("y", literal::s16(CAT('B', 'B')));
  c.assign("z", c.sub("y", "x"));
  c.write("z"); // AB

  c.returnFromFunction();
} c.endFunction();

TEST_END
