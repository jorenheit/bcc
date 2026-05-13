// Decimal print for integer widths and signedness
// Expected: 0 7 255 -1 -3 -128 0 42 65535 -1 -300 -32768

TEST_BEGIN

c.function("main").begin(); {

  // i8 literals
  c.print(literal::i8(0));
  c.write(literal::i8(' '));

  c.print(literal::i8(7));
  c.write(literal::i8(' '));

  c.print(literal::i8(255));
  c.write(literal::i8(' '));

  // s8 literals
  c.print(literal::s8(-1));
  c.write(literal::i8(' '));

  c.print(literal::s8(-3));
  c.write(literal::i8(' '));

  c.print(literal::s8(-128));
  c.write(literal::i8(' '));

  // i16 literals
  c.print(literal::i16(0));
  c.write(literal::i8(' '));

  c.print(literal::i16(42));
  c.write(literal::i8(' '));

  c.print(literal::i16(65535));
  c.write(literal::i8(' '));

  // s16 literals
  c.print(literal::s16(-1));
  c.write(literal::i8(' '));

  c.print(literal::s16(-300));
  c.write(literal::i8(' '));

  c.print(literal::s16(-32768));

  c.returnFromFunction();
} c.endFunction();

TEST_END
