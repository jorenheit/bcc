#pragma once
#include <exception>
#include <sstream>
#include <string>
#include <utility>

namespace error {

  struct Error: std::exception {

    std::string msg;
    Error(std::string const &msgHead = ""): msg(msgHead) {}
    
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
  void throw_if(bool condition, std::string const &filename, int line, int column, Args ... args) {
    if (not condition) return;
    Error err;
    (err << filename << ":" << line << ":" << column << ": " << ... << args);
    throw err;
    std::unreachable();
  }

} // namespace error
