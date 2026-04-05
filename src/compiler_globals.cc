#include "compiler.ih"

void Compiler::fetchGlobal(Slot const &globalSlot, Slot const &localSlot) {
  assert(globalSlot.type == localSlot.type);
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
    copyField(Cell{globalCell, MacroCell::Payload0},
	      Temps<1>::pack(globalCell, MacroCell::Scratch0));
    
    if (useValue1) {
      moveTo(globalCell, MacroCell::Value1); 
      copyField(Cell{globalCell, MacroCell::Payload1},
		Temps<1>::pack(globalCell, MacroCell::Scratch0));
    }

    // Bring payload back
    moveToOriginFrame(useValue1 ? 2 : 1);

    // Move the payload into the local slot  
    moveTo(0, MacroCell::Payload0);
    moveField(Cell{localCell, MacroCell::Value0});
    if (useValue1) {
      moveTo(0, MacroCell::Payload1);
      moveField(Cell{localCell, MacroCell::Value1});
    }
  }
  
  popPtr();
}

void Compiler::putGlobal(Slot const &globalSlot, Slot const &localSlot) {
  assert(globalSlot.type == localSlot.type);
  assert(globalSlot.size() == localSlot.size());

  
  pushPtr();
  bool const useValue1 = globalSlot.type->usesValue1();
  int const size = globalSlot.type->size();

  for (int i = 0; i != size; ++i) {
    int const globalCell = globalSlot + i;
    int const localCell = localSlot + i;
    
    // Load the payload and move it into the global frame
    moveTo(localCell, MacroCell::Value0);
    copyField(Cell{0, MacroCell::Payload0},
	      Temps<1>::pack(0, MacroCell::Scratch0));
    if (useValue1) {
      moveTo(localCell, MacroCell::Value1);
      copyField(Cell{0, MacroCell::Payload1},
		Temps<1>::pack(0, MacroCell::Scratch0));
    }
    
    moveToGlobalFrame(useValue1 ? 2 : 1);

    // Move payload into global slot
    moveTo(0, MacroCell::Payload0);
    moveField(Cell{globalCell, MacroCell::Value0});
    if (useValue1) {
      moveTo(0, MacroCell::Payload1);
      moveField(Cell{globalCell, MacroCell::Value1});
    }
  
    // Return to origin
    moveToOriginFrame();
  }
  
  popPtr();
}

