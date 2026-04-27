#include "builder.ih"

Expression Builder::addressOfImpl(Expression const &obj, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  assert(not obj.isLiteral());
  
  if (obj.slot()->direct()) {
    Slot const slot = obj.slot()->materialize(*this);
    API_REQUIRE(slot.kind != Slot::Temp, "Cannot take the address of a temporary value.");
  }
  
  return Expression(obj.slot()->addressOf(*this));
}


Slot Builder::addressOfSlot(Slot const &slot) {
  assert(slot.kind != Slot::Temp && "taking address of temp");

  types::TypeHandle const pointeeType = slot.type;
  types::TypeHandle const pointerType = TypeSystem::pointer(pointeeType);

  int offset = slot.offset;
  bool localPointer = true;
  if (slot.kind == Slot::Kind::GlobalReference) {
    std::string const globalName = slot.name.substr(std::string("__g_").size());
    assert(_program.isGlobal(globalName));
    Slot const globalSlot = _program.globalSlot(globalName);
    assert(globalSlot.type == pointeeType);
    offset = globalSlot.offset;
    localPointer = false;
    _aliasedGlobals.insert(globalName);
  }

  // Set frame-depth to 0 for a local pointer, FrameID for a global pointer
  Slot const ptrSlot = getTemp(pointerType);
  if (localPointer) {
    moveTo(ptrSlot + RuntimePointer::FrameDepth, MacroCell::Value0);
    zeroCell();
  } else {
    moveTo(0, MacroCell::FrameMarker);
    copyField(Cell{ptrSlot + RuntimePointer::FrameDepth, MacroCell::Value0},
	      Temps<1>::select(ptrSlot + RuntimePointer::FrameDepth, MacroCell::Scratch0));
  }

  // Construct offset in second cell
  moveTo(ptrSlot + RuntimePointer::Offset, MacroCell::Value0);
  setToValue(offset & 0xff);
  moveTo(ptrSlot + RuntimePointer::Offset, MacroCell::Value1);
  setToValue((offset >> 8) & 0xff);

  return ptrSlot;
}

void Builder::copyElementIntoSlot(Slot const &elementSlot, Slot const &arrSlot, Slot const &indexSlot) {
  assert(types::isArray(arrSlot.type));
  assert(types::isInteger(indexSlot.type));
  assert(elementSlot.type == cast<types::ArrayLike>(arrSlot.type)->elementType());
  types::TypeHandle elementType = elementSlot.type;
  
  pushPtr();

  // TODO: use constructive version when I have it
  Slot const scaledIndexSlot = getTemp(TypeSystem::i8());
  assignSlot(scaledIndexSlot, indexSlot);
  moveTo(scaledIndexSlot, MacroCell::Value0);
  mulConst(elementType->size(),
	   Temps<3>::select(scaledIndexSlot, MacroCell::Scratch0,
			    scaledIndexSlot, MacroCell::Scratch1,			  
			    scaledIndexSlot, MacroCell::Payload0
			    ));

  Payload payload(elementType->size(),
		  elementType->usesValue1() ? Payload::Width::Double : Payload::Width::Single);
  
  moveTo(arrSlot, MacroCell::Value0);
  setSeekMarker();
  fetchFromDynamicOffset(Cell{scaledIndexSlot, MacroCell::Value0},
			 Cell{scaledIndexSlot, MacroCell::Value1},
			 payload,
			 primitive::Left);

  for (int i = 0; i != elementType->size(); ++i) {
    moveTo(arrSlot + i, MacroCell::Payload0);
    moveField(Cell{elementSlot + i, MacroCell::Value0});
    if (elementType->usesValue1()) {
      moveTo(arrSlot + i, MacroCell::Payload1);
      moveField(Cell{elementSlot + i, MacroCell::Value1});
    }
  }
  
  moveTo(arrSlot);
  resetSeekMarker();
  popPtr();

}

void Builder::copySlotIntoElement(Slot const &srcSlot, Slot const &arrSlot, Slot const &indexSlot) {
  assert(types::isArray(arrSlot.type));
  assert(types::isInteger(indexSlot.type));

  types::TypeHandle elementType = types::cast<types::ArrayLike>(arrSlot.type)->elementType();
  assert(srcSlot.type == elementType);

  pushPtr();

  Slot const scaledIndexSlot = getTemp(TypeSystem::i8());
  assignSlot(scaledIndexSlot, indexSlot);
  moveTo(scaledIndexSlot, MacroCell::Value0);
  mulConst(elementType->size(),
	   Temps<3>::select(scaledIndexSlot, MacroCell::Scratch0,
			    scaledIndexSlot, MacroCell::Scratch1,			  
			    scaledIndexSlot, MacroCell::Payload0
			    ));
  
  // Plant a seek marker at the start of the array
  moveTo(arrSlot, MacroCell::Value0);
  setSeekMarker();

  // Plant another marker one (full element) beyond the start of the element we need
  goToDynamicOffset(Cell{scaledIndexSlot, MacroCell::Value0},
		    Cell{scaledIndexSlot, MacroCell::Value1});

  _dp.set(0);
  moveTo(elementType->size()); 
  setSeekMarker();
  moveTo(0);

  // Move back to the start of the array
  seek(MacroCell::SeekMarker, primitive::Left, {}, true);
  _dp.set(arrSlot);
  
  for (int i = 0; i != elementType->size(); ++i) {
    // Copy the contents into the payload cells
    moveTo(srcSlot + i, MacroCell::Value0);
    copyField(Cell{arrSlot + i, MacroCell::Payload0},
	      Temps<1>::select(arrSlot + i, MacroCell::Scratch0));
    if (elementType->usesValue1()) {
      moveTo(srcSlot + i, MacroCell::Value1);
      copyField(Cell{arrSlot + i, MacroCell::Payload1},
		Temps<1>::select(arrSlot + i, MacroCell::Scratch0));
    }
  }

  // Move the payload into the cell containing the marker (one beyond actual start of the element)
  moveTo(arrSlot);

  Payload payload(elementType->size(),
		  elementType->usesValue1() ? Payload::Width::Double : Payload::Width::Single);	  
  
  seek(MacroCell::SeekMarker, primitive::Right, payload, false);
  _dp.set(elementType->size());
    
  // Move the payload into the value-cells
  // Pointer value set to the start of the next element, so offset 0 represents the start of the target element
  for (int i = 0; i != elementType->size(); ++i) {
    moveTo(elementType->size() + i, MacroCell::Payload0);
    moveField(Cell{i, MacroCell::Value0});
    if (elementType->usesValue1()) {
      moveTo(elementType->size() + i, MacroCell::Payload1);
      moveField(Cell{i, MacroCell::Value1});
    }
  }

  moveTo(elementType->size());
  resetSeekMarker();

  // Go back to the start of the array
  seek(MacroCell::SeekMarker, primitive::Left, {}, false);
  _dp.set(arrSlot);
  resetSeekMarker();
  popPtr();
}

void Builder::assignSlot(Slot const &dest, Slot const &src) {
  assert(dest.size() >= src.size());

  pushPtr();
  // Copy src into slot
  for (int i = 0; i != dest.size(); ++i) {
    moveTo(src + i, MacroCell::Value0);
    copyField(Cell{dest + i, MacroCell::Value0},
	      Temps<1>::select(dest + i, MacroCell::Scratch0));
    moveTo(src + i, MacroCell::Value1);
    if (dest.type->usesValue1()) {
      copyField(Cell{dest + i, MacroCell::Value1},
		Temps<1>::select(dest + i, MacroCell::Scratch0));
    }
    else {
      setToValue(0);
    }
  }
  popPtr();
}

void Builder::assignSlot(Slot const &slot, values::Literal const &val) {
  pushPtr();
  if (types::isInteger(slot.type)) {
    int const x = values::cast<types::IntegerType>(val)->value();
    moveTo(slot, MacroCell::Value0);
    setToValue(x & 0xff);
    moveTo(slot, MacroCell::Value1);    
    if (slot.type->usesValue1()) {
      setToValue((x >> 8) & 0xff);
    }
    else {
      setToValue(0);      
    }
  }
  else if (types::isArray(slot.type) || types::isString(slot.type)) {
    // recursive call for each element
    for (int i = 0; i != types::cast<types::ArrayLike>(val->type())->length(); ++i) {
      Expression elem = arrayElement(slot, i);
      assert(elem.hasSlot());
      elem.slot()->write(*this, values::cast<types::ArrayLike>(val)->element(i));
    }
  }
  else if (types::isStruct(slot.type)) {
    // recursive call for each field	
    for (int i = 0; i != types::cast<types::StructType>(val->type())->fieldCount(); ++i) {
      Expression field = structField(slot, i);
      assert(field.hasSlot());
      field.slot()->write(*this, values::cast<types::StructType>(val)->field(i));
    }
  }
  else if (types::isFunctionPointer(slot.type)) {
    std::string const &functionName = values::cast<types::FunctionPointerType>(val)->functionName();

    moveTo(slot, MacroCell::Value0); zeroCell();
    emit<primitive::ChangeBy>([functionName](primitive::Context const &ctx) -> int {
      return ctx.getBlockIndex(functionName) & 0xff;
    });

    moveTo(slot, MacroCell::Value1); zeroCell();
    emit<primitive::ChangeBy>([functionName](primitive::Context const &ctx) -> int {
      return (ctx.getBlockIndex(functionName) >> 8) & 0xff;
    });
  }
  else {
    assert(false && "not implemented");
  }
  popPtr();
}


Expression Builder::assignImpl(Expression const &lhs, Expression const &rhs, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_ASSIGNABLE(lhs.type(), rhs.type());

  if (rhs.hasSlot()) {
    lhs.slot()->write(*this, rhs.slot());
  }
  else {
    lhs.slot()->write(*this, rhs.literal());
  }

  return lhs;
}

// TODO: factor out into a writeSlot? That's more consistent with the other API functions
void Builder::writeOutImpl(Expression const &rhs, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();

  pushPtr();

  Slot const slot = rhs.hasSlot()
    ? rhs.slot()->materialize(*this)
    : getTemp(rhs.literal());

  // Special case for Strings: look for NULL terminator
  if (types::isString(slot.type)) {
    moveTo(slot, MacroCell::Value0);
    setSeekMarker();

    emit<primitive::CopyData>(MacroCell::Value0, MacroCell::Flag, MacroCell::Scratch0);
    switchField(MacroCell::Flag);
    loopOpen(); {
      zeroCell();
      switchField(MacroCell::Value0);
      emit<primitive::Out>();
      emit<primitive::MovePointerRelative>(MacroCell::FieldCount);

      // Check if end of string was reached by using the current value as a flag.
      // If NULL terminator hit, we exit the loop and go back to start.
      emit<primitive::CopyData>(MacroCell::Value0, MacroCell::Flag, MacroCell::Scratch0);
      switchField(MacroCell::Flag);
    } loopClose();

    // We hit the end of the string -> return to seek marker (no payload, check current as well)
    seek(MacroCell::SeekMarker, primitive::Left, {}, true);
    resetSeekMarker();
    popPtr();
    return;
  }

  // All other types: just output all Values sequentially
  for (int i = 0; i != slot.type->size(); ++i) {
    moveTo(slot + i, MacroCell::Value0);
    emit<primitive::Out>();
    if (slot.type->usesValue1()) {
      moveTo(slot + i, MacroCell::Value1);
      emit<primitive::Out>();
    }
  }
  popPtr();
}

void Builder::moveToPointee(Slot const &ptrSlot) {
  assert(types::isPointer(ptrSlot.type));
  // NOTE: this leaves the pointer in an unknown position. Leave a seekmarker before calling this
  
  // Decompose the pointer into its frameDepth and offset
  Cell const frameDepth { ptrSlot + RuntimePointer::FrameDepth, MacroCell::Value0 };
  Cell const offsetLow  { ptrSlot + RuntimePointer::Offset, MacroCell::Value0 };
  Cell const offsetHigh { ptrSlot + RuntimePointer::Offset, MacroCell::Value1 };

  // Payload cells start at the origin. First, the pointer-fields
  Cell const frameDepthPayload { 0 + RuntimePointer::FrameDepth, MacroCell::Payload0 };
  Cell const offsetLowPayload  { 0 + RuntimePointer::Offset, MacroCell::Payload0 };
  Cell const offsetHighPayload { 0 + RuntimePointer::Offset, MacroCell::Payload1 };
  
  // Copy pointer (frameDepth and offset) to the payload-cells of cell 0 and 1
  moveTo(frameDepth);
  copyField(frameDepthPayload, Temps<1>::select(frameDepthPayload, MacroCell::Scratch0));

  moveTo(offsetLow);
  copyField(offsetLowPayload,  Temps<1>::select(offsetLowPayload, MacroCell::Scratch0));

  moveTo(offsetHigh);
  copyField(offsetHighPayload, Temps<1>::select(offsetHighPayload, MacroCell::Scratch0));


  // If frameDepth is nonzero, we need to keep moving to the 
  // previous frame start until the depth-counter becomes 0.
  moveTo(frameDepthPayload);
  loopOpen(); {
    Payload payload{
      1, Payload::Width::Single, // depth
      1, Payload::Width::Double, // offset
    };
    moveToPreviousFrame(payload);
    
    // We're now at the start of the previous frame -> exit if depth == 0 after subtracting 1
    moveTo(frameDepthPayload);
    dec();
  } loopClose();

  // At the target frame -> move to offset indicated by pointer value in payload
  goToDynamicOffset(offsetLowPayload, offsetHighPayload);
}


void Builder::writeSlotThroughDereferencedPointer(Slot const &ptrSlot, Slot const &srcSlot) {
  assert(types::isPointer(ptrSlot.type));
  assert(srcSlot.type == types::cast<types::PointerType>(ptrSlot.type)->pointeeType());

  pushPtr();
  // Leave a marker at the sourceSlot and move to pointee
  moveTo(srcSlot);
  setSeekMarker();
  moveToPointee(ptrSlot);

  // Set the marker and move back to the source
  setSeekMarker();
  seek(MacroCell::SeekMarker, primitive::Right, {}, false);
  _dp.set(srcSlot);
  
  // Copy contents of the source-slot into the payload
  for (int i = 0; i != srcSlot.size(); ++i) {
    moveTo(srcSlot + i, MacroCell::Value0);
    copyField(Cell{srcSlot + i, MacroCell::Payload0},
	      Temps<1>::select(srcSlot + i, MacroCell::Scratch0));
    if (srcSlot.type->usesValue1()) {
      moveTo(srcSlot + i, MacroCell::Value1);
      copyField(Cell{srcSlot + i, MacroCell::Payload1},
		Temps<1>::select(srcSlot + i, MacroCell::Scratch0));
    }
  }

  // Seek back to the pointee's slot
  moveTo(srcSlot);
  Payload payload{
    srcSlot.size(),
    srcSlot.type->usesValue1() ? Payload::Width::Double : Payload::Width::Single
  };
  
  seek(MacroCell::SeekMarker, primitive::Left, payload, false);
  resetSeekMarker();
  _dp.set(0);
  
  // Move contents of the payload in the slot
  for (int i = 0; i != srcSlot.size(); ++i) {
    moveTo(i, MacroCell::Payload0);
    moveField(Cell{i, MacroCell::Value0});
    if (srcSlot.type->usesValue1()) {
      moveTo(i, MacroCell::Payload1);
      moveField(Cell{i, MacroCell::Value1});
    }
  }

  // Seek back to the source
  seek(MacroCell::SeekMarker, primitive::Right, {}, false);
  resetSeekMarker();
  _dp.set(srcSlot);
  popPtr();

  syncGlobalToLocal(true);
}



// TODO: factor common code with write
void Builder::dereferencePointerIntoSlot(Slot const &ptrSlot, Slot const &derefSlot) {
  assert(types::isPointer(ptrSlot.type));
  assert(derefSlot.type == types::cast<types::PointerType>(ptrSlot.type)->pointeeType());

  syncLocalToGlobal(true);
  pushPtr();

  // Leave a pointer at the derefSlot and move to the pointee
  moveTo(derefSlot);
  setSeekMarker();
  moveToPointee(ptrSlot);
  _dp.set(0);

  // Copy the value into the payload
  pushPtr();
  for (int i = 0; i != derefSlot.size(); ++i) {
    moveTo(i, MacroCell::Value0);
    copyField(Cell{i, MacroCell::Payload0}, Temps<1>::select(i, MacroCell::Scratch0));
    if (derefSlot.type->usesValue1()) {
      moveTo(i, MacroCell::Value1);    
      copyField(Cell{i, MacroCell::Payload1}, Temps<1>::select(i, MacroCell::Scratch0));
    }
  }
  popPtr();
  
  // Seek back to the start of the frame, then to the seekmarker left behind
  // at the deref-slot. We can't seek to the seekmarker directly because
  // we might have ended up to the right of it, when we're still in the target frame.

  Payload payload {
    derefSlot.size(),
    derefSlot.type->usesValue1() ? Payload::Width::Double : Payload::Width::Single
  };

  seek(MacroCell::FrameMarker, primitive::Left, payload, true); // TODO: this can become false again now that the padding was added to the global frame
  seek(MacroCell::SeekMarker, primitive::Right, payload, false);
  resetSeekMarker();
  
  // We're now at the marker that marks the deref-slot -> need to rebase.
  _dp.set(derefSlot);

  // Move payload into value-cells
  for (int i = 0; i != derefSlot.size(); ++i) {
    moveTo(derefSlot + i, MacroCell::Payload0);
    moveField(Cell{derefSlot + i, MacroCell::Value0});
    if (derefSlot.type->usesValue1()) {
      moveTo(derefSlot + i, MacroCell::Payload1);    
      moveField(Cell{derefSlot + i, MacroCell::Value1});
    } else {
      moveTo(derefSlot + i, MacroCell::Value1);
      zeroCell();
    }
  }
  
  popPtr();
}  


