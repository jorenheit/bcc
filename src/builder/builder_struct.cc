#include "builder.ih"

// Builder::StructDefinition Builder::defineStruct(std::string const& name, API_FUNC) {
//   API_FUNC_BEGIN();
//   API_REQUIRE_INSIDE_PROGRAM_BLOCK();
//   API_REQUIRE_OUTSIDE_FUNCTION_BLOCK();

//   return StructDefinition { *this, name, API_FWD };
// }

// types::TypeHandle Builder::defineStructImpl(std::string const& name, std::vector<types::NameTypePair> const &fields, API_CTX) {
//   types::TypeHandle sType = TypeSystem::defineStruct(name, std::move(fields));
//   API_REQUIRE(sType != nullptr, "conficting struct declaration: '", name, "' previously defined.");
//   return sType;
// }

