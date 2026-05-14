// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <exception>
#include <sstream>
#include <string>
#include <utility>
#include "acus/api/error_codes.h"

namespace acus::error {


  struct Error: std::exception {

    ErrorCode errorCode;
    std::string msg;

    Error(ErrorCode errorCode, std::string const &msgHead = ""):
      errorCode(errorCode), msg(msgHead) {}

    ErrorCode code() const noexcept {
      return errorCode;
    }
    
    template <typename T>
    Error &operator<<(T const &val) {
      std::ostringstream oss;
      oss << val;
      msg += oss.str();
      return *this;
    }

    virtual char const *what() const noexcept override {
      return msg.c_str();
    }
  };

  template <typename ... Args>
  void throw_if(bool condition, ErrorCode errorCode,
                std::string const &filename, int line, int column,
                Args ... args) {
    if (not condition) return;
    Error err(errorCode);
    (err << filename << ":" << line << ":" << column << ": " << ... << args);
    throw err;
    std::unreachable();
  }  

} // namespace acus::error
