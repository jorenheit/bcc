// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Read and write through a pointer to a pointer
// Expected: AZZ

TEST_BEGIN

auto u8   = ts::u8();
auto u8p  = ts::pointer(u8);
auto u8pp = ts::pointer(u8p);

c.function("main").begin(); {
  c.declareLocal("x", u8);
  c.declareLocal("p", u8p);
  c.declareLocal("pp", u8pp);

  c.assign("x", literal::u8('A'));
  c.assign("p", c.addressOf("x"));
  c.assign("pp", c.addressOf("p"));

  auto pFromPp = c.dereferencePointer("pp");
  auto xFromPp = c.dereferencePointer(pFromPp);

  c.write(xFromPp);                 // A

  c.assign(xFromPp, literal::u8('Z'));

  c.write("x");                     // Z
  c.write(c.dereferencePointer("p"));// Z

  c.returnFromFunction();
} c.endFunction();

TEST_END
