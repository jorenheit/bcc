// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests calling function pointers with an u8 argument and an u8 return value.
// The return value is stored in a local supplied to callFunctionPointer and then written out.
// Expected: BA

TEST_BEGIN

auto u8 = ts::u8();
auto fnType = ts::function().ret(u8).param(u8).done();
auto fnPtr = ts::function_pointer(fnType);

c.function("main").begin(); {
  c.declareLocal("fptr", fnPtr);
  c.declareLocal("r", u8);

  c.assign("fptr", literal::function_pointer(fnType, "inc"));
  c.callFunctionPointer("fptr").into("r").arg(literal::u8('A')).done();
  c.write("r");
  c.assign("fptr", literal::function_pointer(fnType, "dec"));
  c.callFunctionPointer("fptr").into("r").arg(literal::u8('B')).done();
  c.write("r");
  c.returnFromFunction();
} c.endFunction();

c.function("inc").param("x", u8).ret(u8).begin(); {
  c.returnFromFunction(c.add("x", literal::u8(1)));
} c.endFunction();

c.function("dec").param("x", u8).ret(u8).begin(); {
  c.returnFromFunction(c.sub("x", literal::u8(1)));
} c.endFunction();

TEST_END
