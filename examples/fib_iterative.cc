// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include "acus/assembler/assembler.h"
using namespace acus;

int main() try {
  Assembler c;

  c.program("fib", "main").begin(); {

    auto u8 = ts::u8();

    c.function("main").begin(); {
      c.declareLocal("n", u8);
      c.declareLocal("i", u8);
      c.declareLocal("a", u8);
      c.declareLocal("b", u8);
      c.declareLocal("next", u8);

      c.assign("n", literal::u8(12)); // Number of iterations

      // fib(0) = 0, fib(1) = 1
      c.assign("i", literal::u8(0));
      c.assign("a", literal::u8(0));
      c.assign("b", literal::u8(1));

      c.label("loop_check");
      c.jumpIf(c.le("i", "n"), "loop_body", "done");
      c.label("loop_body");

      // Output current Fibonacci number.
      c.print("a");
      c.print(literal::string(", "));
      // next = a + b
      c.assign("next", c.add("a", "b"));
      // a = b
      c.assign("a", "b");
      // b = next
      c.assign("b", "next");
      // i++
      c.addAssign("i", literal::u8(1));
      c.jump("loop_check");

      c.label("done");
      c.returnFromFunction();
    } c.endFunction();

    
  } c.endProgram();

  std::cout << c.brainfuck("fib") << '\n';
 } catch (std::exception const &e) {
  std::cerr << e.what() << '\n';
 }
