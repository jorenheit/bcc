#include "compiler.ih"

void Compiler::fetchGlobal(Slot const &globalSlot, Slot const &localSlot) {
  assert(globalSlot.type == localSlot.type);
  assert(globalSlot.size() == localSlot.size());

  pushPtr();
  int const size = globalSlot.type->size();
  bool const useValue1 = globalSlot.type->usesValue1();
  Payload const payload = useValue1 ? Payload::Double : Payload::Single;
  
  for (int i = 0; i != size; ++i) {
    // Move to global frame 
    moveToOrigin();
    setSeekMarker();
    seek(MacroCell::SeekMarker, primitive::Left, payload, false);

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

    // Bring payload back to origin-frame
    seek(MacroCell::SeekMarker, primitive::Right, payload, false);
    resetSeekMarker();
    switchField(static_cast<MacroCell::Field>(0));
    resetOrigin();
    
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

  bool const useValue1 = globalSlot.type->usesValue1();
  int const size = globalSlot.type->size();
  Payload const payload = useValue1 ? Payload::Double : Payload::Single;

  pushPtr();

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

    moveToOrigin();
    setSeekMarker();
    seek(MacroCell::SeekMarker, primitive::Left, payload, false);


    // Move payload into global slot
    moveTo(0, MacroCell::Payload0);
    moveField(Cell{globalCell, MacroCell::Value0});
    if (useValue1) {
      moveTo(0, MacroCell::Payload1);
      moveField(Cell{globalCell, MacroCell::Value1});
    }
  
    // Return to origin
    seek(MacroCell::SeekMarker, primitive::Right, Payload::None, false);
    resetSeekMarker();
    switchField(static_cast<MacroCell::Field>(0));
    resetOrigin();
  }
  
  popPtr();
}

