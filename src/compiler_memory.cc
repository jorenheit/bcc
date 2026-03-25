#include "compiler.ih"


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
	  Slot second {
	    .name = dummyName(),
	    .type = _ts.raw(diff),
	    .kind = Slot::Available,
	    .offset = slot.offset + type->size(),
	    .scope = nullptr
	  };

	  frame.locals.emplace_back(std::move(second));
	  return frame.locals.back();
	}
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
  
  return tryFindAvailableSlot(name, type, kind).value_or(newSlot(name, type, kind));
}

void Compiler::freeSlot(Slot &slot) {
  slot.name = "";
  slot.type = _ts.raw(slot.type->size());
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

Slot Compiler::getTemp(values::Value const &value) {
  Slot tmp = getTemp(value->type());
  assign(lValue(tmp), rValue(value));
  return tmp;
}
