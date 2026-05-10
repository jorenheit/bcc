#include "assembler.ih"

template <auto FetchOrPut>
void Assembler::syncGlobal(Slot const &localSlot, bool onlyAliasedGlobals) {
  assert(localSlot.kind == Slot::GlobalReference);
  
  std::string globalName = localSlot.name.substr(std::string("__g_").size());
  assert(_program.isGlobal(globalName));
  if (onlyAliasedGlobals && not _aliasedGlobals.contains(globalName)) return;
  
  Slot const &globalSlot = _program.globalSlot(globalName);
  assert(globalSlot.size() == localSlot.size());

  (this->*FetchOrPut)(globalSlot, localSlot);
}

template <auto FetchOrPut>
void Assembler::syncGlobals(bool onlyAliasedGlobals) {
  auto const &locals = _currentFunction->frame.locals;
  for (auto const &localSlot: locals) {
    if (localSlot.kind != Slot::GlobalReference) continue;
    syncGlobal<FetchOrPut>(localSlot, onlyAliasedGlobals);
  }  
}

void Assembler::syncGlobalToLocal(bool onlyAliasedGlobals) {
  syncGlobals<&Assembler::fetchGlobal>(onlyAliasedGlobals);
}

void Assembler::syncLocalToGlobal(bool onlyAliasedGlobals) {
  syncGlobals<&Assembler::putGlobal>(onlyAliasedGlobals);
}

void Assembler::fetchGlobal(Slot const &globalSlot, Slot const &localSlot) {
  assert(globalSlot.type == localSlot.type);
  assert(globalSlot.size() == localSlot.size());

  pushPtr();
  int const size = globalSlot.type->size();
  bool const useValue1 = globalSlot.type->usesValue1();

  
  // Move to global frame 
  moveToOrigin();
  setSeekMarker();
  seek(MacroCell::SeekMarker, primitive::Left, {}, false);

  for (int i = 0; i != size; ++i) {
    moveTo(globalSlot + i, MacroCell::Value0);
    copyField(Cell{globalSlot + i, MacroCell::Payload0},
	      Temps<1>::select(globalSlot + i, MacroCell::Scratch0));
    
    if (useValue1) {
      moveTo(globalSlot + i, MacroCell::Value1); 
      copyField(Cell{globalSlot + i, MacroCell::Payload1},
		Temps<1>::select(globalSlot + i, MacroCell::Scratch0));
    }
  }

  // Bring payload back to origin-frame
  Payload payload(size,
		  useValue1 ? Payload::Width::Double : Payload::Width::Single);

  moveTo(globalSlot);
  seek(MacroCell::SeekMarker, primitive::Right, payload, false);
  resetSeekMarker();
  switchField(static_cast<MacroCell::Field>(0));
  resetOrigin();
    
  // Move the payload into the local slot
  for (int i = 0; i != size; ++i) {
    moveTo(i, MacroCell::Payload0);
    moveField(Cell{localSlot + i, MacroCell::Value0});
    if (useValue1) {
      moveTo(i, MacroCell::Payload1);
      moveField(Cell{localSlot + i, MacroCell::Value1});
    }
  }
  
  popPtr();
}

void Assembler::putGlobal(Slot const &globalSlot, Slot const &localSlot) {
  assert(globalSlot.type == localSlot.type);
  assert(globalSlot.size() == localSlot.size());

  bool const useValue1 = globalSlot.type->usesValue1();
  int const size = globalSlot.type->size();

  pushPtr();

  // Copy value into payload slots starting at the origin
  for (int i = 0; i != size; ++i) {
    moveTo(localSlot + i, MacroCell::Value0);
    copyField(Cell{i, MacroCell::Payload0},
	      Temps<1>::select(i, MacroCell::Scratch0));

    if (useValue1) {
      moveTo(localSlot + i, MacroCell::Value1);
      copyField(Cell{i, MacroCell::Payload1},
		Temps<1>::select(i, MacroCell::Scratch0));
    }
  }

  // Move the payload back to the global frame
  Payload payload(size,
		  useValue1 ? Payload::Width::Double : Payload::Width::Single);
   
  moveToOrigin();
  setSeekMarker();
  seek(MacroCell::SeekMarker, primitive::Left, payload, false);
  
  // Move payload into global slot
  for (int i = 0; i != size; ++i) {
    moveTo(i, MacroCell::Payload0);
    moveField(Cell{globalSlot + i, MacroCell::Value0});
    if (useValue1) {
      moveTo(i, MacroCell::Payload1);
      moveField(Cell{globalSlot + i, MacroCell::Value1});
    }
  }
  
  // Return to origin frame
  seek(MacroCell::SeekMarker, primitive::Right, {}, false);
  resetSeekMarker();
  switchField(static_cast<MacroCell::Field>(0));
  resetOrigin();
  
  popPtr();
}


