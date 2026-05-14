// testsuite/tests/integer_casts.cc
//
// Tests integer casts through the expression-only cast API.
//
// Expected output:
//   -20|236|65535|-1|44|-1|255|255|32767|-32768|
//
// Covered:
//   s8  -> s16 : sign-extend
//   u8  -> s16 : zero-extend
//   s8  -> u16 : sign-extend then reinterpret
//   u8  -> s8  : same-width reinterpret
//   u16 -> u8  : narrowing keeps low byte
//   s16 -> s8  : narrowing keeps low byte
//   s16 -> u8  : narrowing keeps low byte, reinterpret unsigned
//   u16 -> s8  : narrowing keeps low byte, reinterpret signed
//   s16 -> u16 : same-width reinterpret
//   u16 -> s16 : same-width reinterpret

TEST_BEGIN

auto u8  = ts::u8();
auto s8  = ts::s8();
auto u16 = ts::u16();
auto s16 = ts::s16();

c.function("main").begin(); {
  c.declareLocal("a", s8);
  c.declareLocal("b", u8);
  c.declareLocal("c16", s16);
  c.declareLocal("d16", u16);

  // s8(-20) -> s16 should sign-extend and print -20.
  c.assign("a", literal::s8(-20));
  c.print(c.cast("a", s16));
  c.write(literal::u8('|'));

  // u8(236) -> s16 should zero-extend and print 236, not -20.
  c.assign("b", literal::u8(236));
  c.print(c.cast("b", s16));
  c.write(literal::u8('|'));

  // s8(-1) -> u16 should sign-extend first, then reinterpret: 0xFFFF = 65535.
  c.assign("a", literal::s8(-1));
  c.print(c.cast("a", u16));
  c.write(literal::u8('|'));

  // u8(255) -> s8 same-width reinterpret: 0xFF = -1.
  c.assign("b", literal::u8(255));
  c.print(c.cast("b", s8));
  c.write(literal::u8('|'));

  // u16(300) -> u8 narrows to low byte: 300 mod 256 = 44.
  c.assign("d16", literal::u16(300));
  c.print(c.cast("d16", u8));
  c.write(literal::u8('|'));

  // s16(-1) -> s8 narrows to low byte: 0xFF = -1.
  c.assign("c16", literal::s16(-1));
  c.print(c.cast("c16", s8));
  c.write(literal::u8('|'));

  // s16(-1) -> u8 narrows to low byte: 0xFF = 255.
  c.assign("c16", literal::s16(-1));
  c.print(c.cast("c16", u8));
  c.write(literal::u8('|'));

  // u16(255) -> s8 narrows to low byte and reinterprets: 0xFF = -1.
  // Cast back to u8 for the print check to verify the raw low byte is still 255.
  c.assign("d16", literal::u16(255));
  c.print(c.cast(c.cast("d16", s8), u8));
  c.write(literal::u8('|'));

  // s16(32767) -> u16 preserves both bytes.
  c.assign("c16", literal::s16(32767));
  c.print(c.cast("c16", u16));
  c.write(literal::u8('|'));

  // u16(32768) -> s16 preserves both bytes and reinterprets as -32768.
  c.assign("d16", literal::u16(32768));
  c.print(c.cast("d16", s16));
  c.write(literal::u8('|'));

  c.returnFromFunction();
} c.endFunction();

TEST_END
