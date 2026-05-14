// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests an array of function pointers indexed with static indices.
// arr[1] is called first, then arr[0], so storage order and selected target are both checked.
// Expected: BA

TEST_BEGIN

auto voidT = ts::void_t();
auto fnType = ts::function().ret(voidT).done();
auto fnPtr = ts::function_pointer(fnType);
auto fnArray = ts::array(fnPtr, 2);

c.function("main").begin(); {
  c.declareLocal("arr", fnArray);

  c.assign(c.arrayElement("arr", 0), literal::function_pointer(fnType, "printA"));
  c.assign(c.arrayElement("arr", 1), literal::function_pointer(fnType, "printB"));
  c.callFunctionPointer(c.arrayElement("arr", 1)).done();
  c.callFunctionPointer(c.arrayElement("arr", 0)).done();
  c.returnFromFunction();
} c.endFunction();

c.function("printA").begin(); {
  c.write(literal::u8('A'));
  c.returnFromFunction();
} c.endFunction();

c.function("printB").begin(); {
  c.write(literal::u8('B'));
  c.returnFromFunction();
} c.endFunction();

TEST_END
