// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include "acus/assembler/assembler.h"
using namespace acus;


int main() try {
  Assembler c;

  c.program("hello", "main").begin(); {

    c.function("main").begin(); {
      c.declareLocal("x", ts::u8());
      c.label("a");
      for (int i = 0; i != 256; ++i) {
	c.assign("x", literal::u8(i));
	c.print("x");
	c.write(literal::u8('\n'));
      }
      c.returnFromFunction();
    } c.endFunction();

  } c.endProgram();
 
  std::cout << c.brainfuck("hello") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
