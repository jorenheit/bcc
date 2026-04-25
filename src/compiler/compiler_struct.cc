#include "compiler.ih"

Compiler::StructDefinition Compiler::defineStruct(std::string const& name, API_FUNC) {
  API_FUNC_BEGIN("defineStruct");
  API_REQUIRE_INSIDE_PROGRAM_BLOCK();
  API_REQUIRE_OUTSIDE_FUNCTION_BLOCK();

  return StructDefinition {
    ._compiler = this,
    ._structName = name,
    ._context = API_FWD
  };
}

types::TypeHandle Compiler::defineStructImpl(std::string const& name, StructFields const &fields, API_CTX) {
  types::TypeHandle sType = TypeSystem::defineStruct(name, std::move(fields));
  API_REQUIRE(sType != nullptr, "conficting struct declaration: '", name, "' previously defined.");
  return sType;
}

