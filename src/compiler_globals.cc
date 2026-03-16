#include "compiler.ih"

void Compiler::fetchGlobal(Slot const &globalSlot, Slot const &localSlot) {
  assert(types::match(*globalSlot.type, *localSlot.type));
  assert(globalSlot.size() == localSlot.size());

  
  pushPtr();
  bool const useValue1 = globalSlot.type->usesValue1();
  
  // Move to global frame and load the data into the payload field
  moveToGlobalFrame();

  moveTo(globalSlot, MacroCell::Value0); // TODO: handle size > 1, TODO: Value1
  copyField(globalSlot, MacroCell::Payload0);
  if (useValue1) {
    moveTo(globalSlot, MacroCell::Value1); 
    copyField(globalSlot, MacroCell::Payload1);
  }

  // Bring payload back
  moveToOriginFrame(useValue1 ? 2 : 1);

  // Clear local slot
  moveTo(localSlot, MacroCell::Value0); zeroCell();
  moveTo(localSlot, MacroCell::Value1); zeroCell();

  // Move the payload into the local slot
  moveTo(0, MacroCell::Payload0);
  moveField(localSlot, MacroCell::Value0);
  if (useValue1) {
    moveTo(0, MacroCell::Payload1);
    moveField(localSlot, MacroCell::Value1);    
  }
  
  popPtr();
}

void Compiler::putGlobal(Slot const &globalSlot, Slot const &localSlot) {
  assert(types::match(*globalSlot.type, *localSlot.type));
  assert(globalSlot.size() == localSlot.size());

  pushPtr();
  bool const useValue1 = globalSlot.type->usesValue1();

  // Load the payload int and move it into the global frame
  moveTo(localSlot, MacroCell::Value0);
  copyField(0, MacroCell::Payload0);
  if (useValue1) {
    moveTo(localSlot, MacroCell::Value1);
    copyField(0, MacroCell::Payload1);
  }

  moveToGlobalFrame(useValue1 ? 2 : 1);

  // Clear the target slot
  moveTo(globalSlot, MacroCell::Value0);
  zeroCell();

  // Move payload into target
  moveTo(0, MacroCell::Payload0);
  moveField(globalSlot, MacroCell::Value0);
  if (useValue1) {
    moveTo(0, MacroCell::Payload1);
    moveField(globalSlot, MacroCell::Value1);
  }
  
  // Return to origin
  moveToOriginFrame();

  popPtr();
}

void Compiler::syncGlobalToLocal() {
  auto const &locals = _currentFunction->frame.locals;
  auto const &globals = _program.globals;
      
  for (auto const &[localName, localSlot]: locals) {
    if (localSlot.storageType != Slot::GlobalReference) continue;
    std::string globalName = localName.substr(std::string("__g_").size());
    assert(globals.contains(globalName));

    Slot const &globalSlot = globals.at(globalName);
    assert(globalSlot.size() == localSlot.size());

    fetchGlobal(globalSlot, localSlot);
  }  
}


void Compiler::syncLocalToGlobal() {
  auto const &locals = _currentFunction->frame.locals;
  auto const &globals = _program.globals;
      
  for (auto const &[localName, localSlot]: locals) {
    if (localSlot.storageType != Slot::GlobalReference) continue;
    std::string globalName = localName.substr(std::string("__g_").size());
    assert(globals.contains(globalName));

    Slot const &globalSlot = globals.at(globalName);
    assert(globalSlot.size() == localSlot.size());

    putGlobal(globalSlot, localSlot);
  }  
}
