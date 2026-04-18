#pragma once
#include <source_location>
#include <string>


class Compiler;

namespace api {

  class Context {
    Compiler const *_compiler;
    std::string _name;
    std::source_location _loc;
    
  public:
    Context() = default;
    Context(Compiler const &c, std::string const &name, std::source_location loc);

    bool programStarted() const;
    std::string currentFunction() const;
    std::string currentBlock() const;
    int currentScopeDepth() const;
    bool declaredAsGlobal(std::string const &name) const;
    bool inScope(std::string const &name) const;
    bool inCurrentScope(std::string const &name) const;
    std::string apiName() const;
    bool globalDeclarationsAllowed() const;

    inline auto file_name() const { return _loc.file_name(); }
    inline auto function_name() const { return _loc.function_name(); }
    inline auto line() const { return _loc.line(); }
    inline auto column() const { return _loc.column(); }
  };
  
} // namespace api
