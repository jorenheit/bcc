#pragma once
#include <source_location>
#include <string>

namespace acus {
  class Builder;
}

namespace acus::api::impl {

  class Context {
    Builder const *_builder = nullptr;
    std::string _name;
    std::source_location _loc;
    
  public:
    Context() = default;
    Context(std::string const &name, std::source_location loc);    
    Context(Builder const &c, std::string const &name, std::source_location loc);

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
  
} // namespace api::impl
