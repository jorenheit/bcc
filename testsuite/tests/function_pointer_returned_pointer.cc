// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests returning function pointers from another function and calling both returned targets in one program.
// This stresses function-pointer return transport and subsequent indirect calls.
// Expected: TF

TEST_BEGIN

auto u8 = ts::u8();
auto voidT = ts::void_t();
auto targetType = ts::function().ret(voidT).done();
auto targetPtr = ts::function_pointer(targetType);

c.function("main").begin(); {
  c.declareLocal("fptr", targetPtr);

  c.callFunction("choose").into("fptr").arg(literal::u8(1)).done();
  c.callFunctionPointer("fptr").done();
  c.callFunction("choose").into("fptr").arg(literal::u8(0)).done();
  c.callFunctionPointer("fptr").done();
  c.returnFromFunction();
} c.endFunction();

c.function("choose").param("flag", u8).ret(targetPtr).begin(); {
  c.jumpIf("flag", "true", "false");
  c.label("true");
  c.returnFromFunction(literal::function_pointer(targetType, "printT"));
  c.label("false");
  c.returnFromFunction(literal::function_pointer(targetType, "printF"));
} c.endFunction();

c.function("printT").begin(); {
  c.write(literal::u8('T'));
  c.returnFromFunction();
} c.endFunction();

c.function("printF").begin(); {
  c.write(literal::u8('F'));
  c.returnFromFunction();
} c.endFunction();

TEST_END
