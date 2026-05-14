// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Subtract u16 array elements using sub and subAssign
// Expected: GADAAA

TEST_BEGIN

auto array3 = ts::array(ts::u16(), 3);

c.function("main").begin(); {
  c.declareLocal("arr", array3);
  c.declareLocal("z", ts::u16());

  c.assign(c.arrayElement("arr", 0), literal::u16(CAT('G', 'A')));
  c.assign(c.arrayElement("arr", 1), literal::u16(3));

  auto x = c.arrayElement("arr", 0);
  auto y = c.arrayElement("arr", 1);

  c.write(x);              // GA
  c.subAssign(x, y);          // arr[0] -= arr[1]
  c.write(x);              // DA
  c.assign("z", c.sub(x, y)); // z = arr[0] - arr[1]
  c.write("z");            // AA

  c.returnFromFunction();
} c.endFunction();

TEST_END
