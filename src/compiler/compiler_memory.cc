#include "compiler.ih"


// TODO: does not belong in public interface
Slot Compiler::local(std::string const& varName, bool globalReference) const {
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


Slot Compiler::allocSlot(std::string const &name, types::TypeHandle type, Slot::Kind kind) {

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

	// Split the slot if there is still room
	auto const dummyName = []() {
	  static int counter = 0; return std::string("__dummy_") + std::to_string(counter++);
	};
	
	if (diff > 0) {
	  frame.locals.emplace_back(Slot{
	      .name = dummyName(),
	      .type = TypeSystem::raw(diff),
	      .kind = Slot::Available,
	      .offset = slot.offset + type->size(),
	      .scope = nullptr
	    });
	}
	return slot;
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

void Compiler::freeSlot(Slot &slot) {
  slot.name = "";
  slot.type = TypeSystem::raw(slot.type->size());
  slot.kind = Slot::Available;
  slot.scope = nullptr;
}

void Compiler::freeTemps() {
  for (auto &slot: _currentFunction->frame.locals) {
    if (slot.kind == Slot::Temp) freeSlot(slot);
  }
}

void Compiler::freeScope(Function::Scope const *scope) {
  for (auto &slot: _currentFunction->frame.locals) {
    if (slot.scope == scope) {
      freeSlot(slot);
    }
  }
}

Slot Compiler::getTemp(types::TypeHandle type) {
  assert(_currentBlock != nullptr);
  return allocSlot("__tmp", type, Slot::Temp);
}

Slot Compiler::getTemp(values::Literal const &value) {
  Slot tmp = getTemp(value->type());
  assignSlot(tmp, value);
  return tmp;
}
