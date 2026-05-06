// Decimal print for integer variables with different widths and signedness
// Expected: 255 -3 65535 -300 127 -128 32767 -32768

TEST_BEGIN

c.function("main").begin(); {
  c.declareLocal("u8", ts::i8());
  c.declareLocal("s8", ts::s8());
  c.declareLocal("u16", ts::i16());
  c.declareLocal("s16", ts::s16());

  c.assign("u8", literal::i8(255));
  c.print("u8");
  c.writeOut(literal::i8(' '));

  c.assign("s8", literal::s8(-3));
  c.print("s8");
  c.writeOut(literal::i8(' '));

  c.assign("u16", literal::i16(65535));
  c.print("u16");
  c.writeOut(literal::i8(' '));

  c.assign("s16", literal::s16(-300));
  c.print("s16");
  c.writeOut(literal::i8(' '));

  c.assign("s8", literal::s8(127));
  c.print("s8");
  c.writeOut(literal::i8(' '));

  c.assign("s8", literal::s8(-128));
  c.print("s8");
  c.writeOut(literal::i8(' '));

  c.assign("s16", literal::s16(32767));
  c.print("s16");
  c.writeOut(literal::i8(' '));

  c.assign("s16", literal::s16(-32768));
  c.print("s16");

  c.returnFromFunction();
} c.endFunction();

TEST_END
