#include "assembler.ih"

Slot Assembler::local(std::string const& varName, bool globalReference) const {
  assert(_currentFunction != nullptr);

  Function::Scope *targetScope = _currentScope;
  while (true) {
    for (auto &slot: _currentFunction->frame.locals) {
      if (slot.name == varName && slot.scope == targetScope) {
	return slot;
      }
    }
    if (targetScope == nullptr) break;
    targetScope = targetScope->parent;
  }

  if (not globalReference) {
    std::string const globalReferenceName = std::string("__g_") + varName;
    return local(globalReferenceName, true);
  }

  return Slot::invalid();
}


Slot Assembler::allocSlot(std::string const &name, types::TypeHandle type, Slot::Kind kind) {

  assert(_currentFunction != nullptr);
  
  auto const tryFindAvailableSlot = [&](std::string const &name,
				        types::TypeHandle type,
				        Slot::Kind kind) -> std::optional<Slot> {
    
    // Now check if there is an existing slot that fits this type
    auto &frame = _currentFunction->frame;  
    for (auto &slot: frame.locals) {
      if (slot.kind == Slot::Available && slot.type->size() >= type->size()) {
	int const diff = slot.type->size() - type->size();

	// Reuse this slot
	slot.name = name;
	slot.type = type;
	slot.kind = kind;
	slot.scope = _currentScope;

	// Store the slot in case the reference is invalidated below
	Slot const result = slot;

	// Split the slot if there is still room
	auto const dummyName = []() {
	  static int counter = 0; return std::string("__dummy_") + std::to_string(counter++);
	};

	if (diff > 0) {
	  // Might invalidate the reference -> use result below
	  frame.locals.push_back(Slot{
	      .name = dummyName(),
	      .type = ts::raw(diff),
	      .kind = Slot::Available,
	      .offset = slot.offset + slot.type->size(),
	      .scope = nullptr
	    });
	}
	return result;
      }
    }
    return {};
  };

  auto const newSlot = [&](std::string const &name,
			   types::TypeHandle type,
			   Slot::Kind kind) -> Slot {

    auto &frame = _currentFunction->frame;
    Slot newSlot {
      .name = name,
      .type = type,
      .kind = kind,
      .offset = frame.localBase() + frame.localAreaSize(),
      .scope = _currentScope
    };
    
    frame.locals.emplace_back(std::move(newSlot));
    return frame.locals.back();
  };
  
  auto opt = tryFindAvailableSlot(name, type, kind);
  if (opt) return *opt;
  return newSlot(name, type, kind);
}


void Assembler::freeSlot(Slot &slot) {
  slot.name = "";
  slot.type = ts::raw(slot.type->size());
  slot.kind = Slot::Available;
  slot.scope = nullptr;
}

void Assembler::freeTemp(Slot const &target) {
  assert(target.kind == Slot::Temp);
  
  // Find this slot in the current frame and free it.
  for (auto &slot: _currentFunction->frame.locals) {
    if (slot == target) {
      freeSlot(slot);
      return;
    }
  }
  // Ignore. The temp slot might be a subslot of a larger materialized temp
  // Not sure if we should free the entire slot in that case, seems risky.
  // Just leave (leak) it for now, it will be freed eventually anyway.
}


void Assembler::freeTemps() {
  for (auto &slot: _currentFunction->frame.locals) {
    if (slot.kind == Slot::Temp) freeSlot(slot);
  }
}

void Assembler::freeScope(Function::Scope const *scope) {
  for (auto &slot: _currentFunction->frame.locals) {
    if (slot.scope == scope) {
      freeSlot(slot);
    }
  }
}

Slot Assembler::getTemp(types::TypeHandle type) {
  assert(_currentBlock != nullptr);
  return allocSlot("__tmp", type, Slot::Temp);
}

Slot Assembler::getTemp(literal::Literal const &value) {
  Slot tmp = getTemp(value->type());
  assignSlot(tmp, value);
  return tmp;
}

Slot Assembler::declareGlobal(std::string const &name, types::TypeHandle type, API_FUNC) {
  API_FUNC_BEGIN();
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


Slot Assembler::declareLocal(std::string const& name, types::TypeHandle type, API_FUNC) {
  API_FUNC_BEGIN();
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_FUNCTION_BLOCK();
  API_REQUIRE_NOT_IN_CURRENT_SCOPE(name);

  return allocSlot(name, type, Slot::Local);
}

Slot Assembler::declareGlobalReference(Slot const &globalSlot) {
  assert(globalSlot.kind == Slot::Global);
  assert(_currentFunction != nullptr);
  assert(_currentBlock != nullptr);
  
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


void Assembler::referGlobals(std::vector<std::string> const &names, API_FUNC) {
  API_FUNC_BEGIN();
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_FUNCTION_BLOCK();
  
  std::unordered_set<std::string> declared;
  for (std::string const &name: names) {
    API_REQUIRE_IS_GLOBAL(name);

    auto [_, unique] = declared.insert(name);
    API_REQUIRE(unique,
		error::ErrorCode::DuplicateGlobalReferences,
		"multiple references to ", name, ".");
    declareGlobalReference(_program.globalSlot(name));      
  }

  syncGlobalToLocal();
}
