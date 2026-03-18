#include "compiler.ih"

// TODO: verify that size>1 works as expected

void Compiler::fetchGlobal(Slot const &globalSlot, Slot const &localSlot) {
  assert(types::match(globalSlot.type, localSlot.type));
  assert(globalSlot.size() == localSlot.size());

  pushPtr();
  bool const useValue1 = globalSlot.type->usesValue1();
  int const size = globalSlot.type->size();

  for (int i = 0; i != size; ++i) {
    // Move to global frame and load the data into the payload field
    moveToGlobalFrame();

    int const globalCell = globalSlot + i;
    int const localCell = localSlot + i;
    
    moveTo(globalCell, MacroCell::Value0);
    copyField(globalCell, MacroCell::Payload0);
    if (useValue1) {
      moveTo(globalCell, MacroCell::Value1); 
      copyField(globalCell, MacroCell::Payload1);
    }

    // Bring payload back
    moveToOriginFrame(useValue1 ? 2 : 1);

    // Move the payload into the local slot 
    moveTo(localCell, MacroCell::Value0); // TODO: factor this out into assign
    zeroCell();
    moveTo(0, MacroCell::Payload0);
    moveField(localCell, MacroCell::Value0);
    if (useValue1) {
      moveTo(localCell, MacroCell::Value1);
      zeroCell();
      moveTo(0, MacroCell::Payload1);
      moveField(localCell, MacroCell::Value1);    
    }
  }
  
  popPtr();
}

void Compiler::putGlobal(Slot const &globalSlot, Slot const &localSlot) {
  assert(types::match(globalSlot.type, localSlot.type));
  assert(globalSlot.size() == localSlot.size());

  
  pushPtr();
  bool const useValue1 = globalSlot.type->usesValue1();
  int const size = globalSlot.type->size();

  for (int i = 0; i != size; ++i) {
    int const globalCell = globalSlot + i;
    int const localCell = localSlot + i;

    
    // Load the payload int and move it into the global frame
    moveTo(localCell, MacroCell::Value0);
    copyField(0, MacroCell::Payload0);
    if (useValue1) {
      moveTo(localCell, MacroCell::Value1);
      copyField(0, MacroCell::Payload1);
    }

    moveToGlobalFrame(useValue1 ? 2 : 1);

    // Move payload into global slot
    moveTo(globalCell, MacroCell::Value0);
    zeroCell();
    moveTo(0, MacroCell::Payload0);
    moveField(globalCell, MacroCell::Value0);
    if (useValue1) {
      moveTo(globalCell, MacroCell::Value1);
      zeroCell();
      moveTo(0, MacroCell::Payload1);
      moveField(globalCell, MacroCell::Value1);
    }
  
    // Return to origin
    moveToOriginFrame();
  }
  
  popPtr();
}

