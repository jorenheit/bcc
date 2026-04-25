#include "compiler.ih"

Slot Compiler::declareGlobal(std::string const &name, types::TypeHandle type, API_FUNC) {
  API_FUNC_BEGIN("declareGlobal");
  API_CHECK_EXPECTED();
  API_REQUIRE_DECLARE_GLOBAL_ALLOWED();
  API_REQUIRE_INSIDE_PROGRAM_BLOCK();
  API_REQUIRE_OUTSIDE_FUNCTION_BLOCK();
  API_REQUIRE_GLOBAL_NAME_AVAILABLE(name);

  int const offset = _program.globalVariableFrameSize();
  Slot slot {
    .name = name,
    .type = type,
    .kind = Slot::Global,
    .offset = offset,
    .scope = nullptr
  };

  _program.globals.emplace_back(slot);
  return slot;
}


Slot Compiler::declareLocal(std::string const& name, types::TypeHandle type, API_FUNC) {
  API_FUNC_BEGIN("declareLocal");
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_FUNCTION_BLOCK();
  API_REQUIRE_OUTSIDE_CODE_BLOCK();
  API_REQUIRE_NOT_IN_CURRENT_SCOPE(name);

  return allocSlot(name, type, Slot::Local);
}


// TODO: does not belong in public interface
Slot Compiler::declareGlobalReference(Slot const &globalSlot) {
  assert(globalSlot.kind == Slot::Global);
  assert(_currentFunction != nullptr);
  assert(_currentBlock != nullptr && _currentBlock->name.starts_with("__prologue_"));
  
  FrameLayout &frame = _currentFunction->frame;
  int const offset = frame.localBase() + frame.localAreaSize();
  Slot slot {
    .name = std::string("__g_") + globalSlot.name,
    .type = globalSlot.type,
    .kind = Slot::GlobalReference,
    .offset = offset,
    .scope = _currentScope
  };
  frame.locals.emplace_back(std::move(slot));
  return frame.locals.back();
}


void Compiler::referGlobals(std::vector<std::string> const &names, API_FUNC) {
  API_FUNC_BEGIN("referGlobals");
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_FUNCTION_BLOCK();
  API_REQUIRE_OUTSIDE_CODE_BLOCK();
  
  beginBlock(std::string("__prologue_") + _currentFunction->name); {

    std::unordered_set<std::string> declared;
    for (std::string const &name: names) {
      API_REQUIRE_IS_GLOBAL(name);

      auto [_, unique] = declared.insert(name);
      API_REQUIRE(unique, "multiple references to ", name, ".");
      declareGlobalReference(_program.globalSlot(name));      
    }

    syncGlobalToLocal();
    setNextBlockImpl(_program.nextGlobalBlockIndex());
    API_EXPECT_NEXT("endBlock");
  } endBlock();
}

types::TypeHandle Compiler::defineStruct(std::string const& name, StructFields const &fields, API_FUNC) {
  API_FUNC_BEGIN("defineStruct");
  API_REQUIRE_INSIDE_PROGRAM_BLOCK();
  API_REQUIRE_OUTSIDE_FUNCTION_BLOCK();
    
  types::TypeHandle sType = TypeSystem::defineStruct(name, std::move(fields));
  API_REQUIRE(sType != nullptr, "conficting struct declaration: '", name, "' previously defined.");
  return sType;
}

