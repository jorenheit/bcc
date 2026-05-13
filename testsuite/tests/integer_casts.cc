// testsuite/tests/integer_casts.cc
//
// Tests integer casts through the expression-only cast API.
//
// Expected output:
//   -20|236|65535|-1|44|-1|255|255|32767|-32768|
//
// Covered:
//   s8  -> s16 : sign-extend
//   i8  -> s16 : zero-extend
//   s8  -> i16 : sign-extend then reinterpret
//   i8  -> s8  : same-width reinterpret
//   i16 -> i8  : narrowing keeps low byte
//   s16 -> s8  : narrowing keeps low byte
//   s16 -> i8  : narrowing keeps low byte, reinterpret unsigned
//   i16 -> s8  : narrowing keeps low byte, reinterpret signed
//   s16 -> i16 : same-width reinterpret
//   i16 -> s16 : same-width reinterpret

TEST_BEGIN

auto i8  = ts::i8();
auto s8  = ts::s8();
auto i16 = ts::i16();
auto s16 = ts::s16();

c.function("main").begin(); {
  c.declareLocal("a", s8);
  c.declareLocal("b", i8);
  c.declareLocal("c16", s16);
  c.declareLocal("d16", i16);

  // s8(-20) -> s16 should sign-extend and print -20.
  c.assign("a", literal::s8(-20));
  c.print(c.cast("a", s16));
  c.write(literal::i8('|'));

  // i8(236) -> s16 should zero-extend and print 236, not -20.
  c.assign("b", literal::i8(236));
  c.print(c.cast("b", s16));
  c.write(literal::i8('|'));

  // s8(-1) -> i16 should sign-extend first, then reinterpret: 0xFFFF = 65535.
  c.assign("a", literal::s8(-1));
  c.print(c.cast("a", i16));
  c.write(literal::i8('|'));

  // i8(255) -> s8 same-width reinterpret: 0xFF = -1.
  c.assign("b", literal::i8(255));
  c.print(c.cast("b", s8));
  c.write(literal::i8('|'));

  // i16(300) -> i8 narrows to low byte: 300 mod 256 = 44.
  c.assign("d16", literal::i16(300));
  c.print(c.cast("d16", i8));
  c.write(literal::i8('|'));

  // s16(-1) -> s8 narrows to low byte: 0xFF = -1.
  c.assign("c16", literal::s16(-1));
  c.print(c.cast("c16", s8));
  c.write(literal::i8('|'));

  // s16(-1) -> i8 narrows to low byte: 0xFF = 255.
  c.assign("c16", literal::s16(-1));
  c.print(c.cast("c16", i8));
  c.write(literal::i8('|'));

  // i16(255) -> s8 narrows to low byte and reinterprets: 0xFF = -1.
  // Cast back to i8 for the print check to verify the raw low byte is still 255.
  c.assign("d16", literal::i16(255));
  c.print(c.cast(c.cast("d16", s8), i8));
  c.write(literal::i8('|'));

  // s16(32767) -> i16 preserves both bytes.
  c.assign("c16", literal::s16(32767));
  c.print(c.cast("c16", i16));
  c.write(literal::i8('|'));

  // i16(32768) -> s16 preserves both bytes and reinterprets as -32768.
  c.assign("d16", literal::i16(32768));
  c.print(c.cast("d16", s16));
  c.write(literal::i8('|'));

  c.returnFromFunction();
} c.endFunction();

TEST_END
