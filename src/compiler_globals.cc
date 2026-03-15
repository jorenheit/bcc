#include "compiler.ih"

void Compiler::fetchGlobal(Slot const &globalSlot, Slot const &localSlot) {
  pushPtr();

  // Move to global frame and load the data into the payload field
  moveToGlobalFrame();
  moveTo(globalSlot, MacroCell::Value0); // TODO: handle size > 1, TODO: Value1
  copyField(globalSlot, MacroCell::Payload0);

  // Bring payload back
  moveToOriginFrameWithPayload();

  // Clear local slot
  moveTo(localSlot, MacroCell::Value0);
  zeroCell();

  // Move the payload into the local slot
  moveTo(0, MacroCell::Payload0);
  moveField(localSlot, MacroCell::Value0);

  popPtr();
}

void Compiler::putGlobal(Slot const &globalSlot, Slot const &localSlot) {
  pushPtr();

  // Load the payload int and move it into the global frame
  moveTo(localSlot, MacroCell::Value0);
  copyField(0, MacroCell::Payload0);
  moveToGlobalFrameWithPayload();

  // Clear the target slot
  moveTo(globalSlot, MacroCell::Value0);
  zeroCell();

  // Move payload into target
  moveTo(0, MacroCell::Payload0);
  moveField(globalSlot, MacroCell::Value0);

  // Return to origin
  moveToOriginFrame();

  popPtr();
}

void Compiler::syncGlobalToLocal() {
  auto const &locals = _currentFunction->frame.locals;
  auto const &globals = _program.globals;
      
  for (auto const &[localName, localSlot]: locals) {
    if (localSlot.type != Slot::GlobalReference) continue;
    std::string globalName = localName.substr(std::string("__g_").size());
    assert(globals.contains(globalName));

    Slot const &globalSlot = globals.at(globalName);
    assert(globalSlot.size == localSlot.size);

    fetchGlobal(globalSlot, localSlot);
  }  
}


void Compiler::syncLocalToGlobal() {
  auto const &locals = _currentFunction->frame.locals;
  auto const &globals = _program.globals;
      
  for (auto const &[localName, localSlot]: locals) {
    if (localSlot.type != Slot::GlobalReference) continue;
    std::string globalName = localName.substr(std::string("__g_").size());
    assert(globals.contains(globalName));

    Slot const &globalSlot = globals.at(globalName);
    assert(globalSlot.size == localSlot.size);

    putGlobal(globalSlot, localSlot);
  }  
}
