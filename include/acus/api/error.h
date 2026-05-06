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

    Error(ErrorCode errorCode = ErrorCode::GenericApiRequirement,
          std::string const &msgHead = ""):
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

  template <typename ... Args>
  void throw_if(bool condition,
                std::string const &filename, int line, int column,
                Args ... args) {
    throw_if(condition, ErrorCode::GenericApiRequirement,
             filename, line, column, args...);
  }

} // namespace acus::error
