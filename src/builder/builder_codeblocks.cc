#include "builder.ih"

void Builder::blockOpen() {
  assert(_dp.current().offset == 0);
  assert(_currentBlock != nullptr);

  // To start a block, we need to check 2 conditions:
  // 1. Does the block-index match the value stored in the TargetBlock cell?
  // 2. Is the Run-cell still set?
  //
  // If both are true, the Flag field of the TargetBlock cell is set to 1 and
  // used as the conditional cell upon which it is decided whether or not to enter
  // the block.

  moveTo(FrameLayout::TargetBlock, MacroCell::Value0);
  compare16ToConstConstructive(/* value =    */ _currentBlock->globalBlockIndex,
			       /* highByte = */ Cell{FrameLayout::TargetBlock, MacroCell::Value1},
			       /* result =   */ Cell{FrameLayout::TargetBlock, MacroCell::Flag},
			       Temps<2>::select(FrameLayout::TargetBlock, MacroCell::Scratch0,
						FrameLayout::TargetBlock, MacroCell::Scratch1));

  moveTo(FrameLayout::RunState, MacroCell::Value0);
  andConstructive(/* result = */ Cell{FrameLayout::RunState, MacroCell::Flag},
		  /* other  = */ Cell{FrameLayout::TargetBlock, MacroCell::Flag},
		  Temps<2>::select(FrameLayout::RunState, MacroCell::Scratch0,
				   FrameLayout::RunState, MacroCell::Scratch1));

  // Clear the targetblock flag
  moveTo(FrameLayout::TargetBlock, MacroCell::Flag);
  zeroCell();

  // Open the block, conditional on the run-flag
  moveTo(FrameLayout::RunState, MacroCell::Flag);
  loopOpen("block open");
  zeroCell();
  moveToOrigin();
}

void Builder::blockClose() {
  assert(_currentBlock != nullptr);

  // We move back to the Flag stored in the RunState cell (guaranteed zero)
  moveTo(FrameLayout::RunState, MacroCell::Flag);
  loopClose("block close");
  moveToOrigin();
}

void Builder::constructMetaBlocks() {
  assert(_currentFunction == nullptr);
  assert(_currentBlock == nullptr);

  for (size_t idx = 0; idx != _metaBlocks.size(); ++idx) {
    MetaBlock const &m = _metaBlocks[idx];
    types::TypeHandle returnType = std::holds_alternative<std::string>(m.callee)
      ? _program.function(std::get<std::string>(m.callee)).type->returnType()
      : std::get<types::FunctionType const *>(m.callee)->returnType();

    // Set current function to caller (owner of metablock) and construct block
    _currentFunction = &_program.function(m.caller);    
    beginBlock(m.name); {

      if (returnType == ts::voidT() || not m.returnSlot){
	fetchReturnData();
      }
      else {
	SlotProxy returnSlot = *m.returnSlot;
	assert(returnType == returnSlot->type());	

	if (returnSlot->direct()) {
	  Slot const ret = returnSlot->materialize(*this);
	  fetchReturnData(ret);
	  if (ret.kind == Slot::GlobalReference) {
	    syncGlobal<&Builder::putGlobal>(ret);
	  }
	}
	else {
	  Slot const ret = getTemp(returnType);
	  fetchReturnData(ret);
	  returnSlot->write(*this, ret);
	}
      }

      // Check if the run-state has become 0. If so, unwind the stack
      moveTo(FrameLayout::RunState, MacroCell::Value0);
      copyField(Cell{FrameLayout::RunState, MacroCell::Scratch0},
		Temps<1>::select(_dp.current().offset, MacroCell::Scratch1)); 
      moveTo(FrameLayout::RunState, MacroCell::Scratch1);
      setToValue(1);
      
      switchField(MacroCell::Scratch0);
      loopOpen(); { // if run: sync globals and set next block
	zeroCell();
	switchField(MacroCell::Scratch1);
	zeroCell();
	syncGlobalToLocal();
	setNextBlock(m.caller, m.nextBlockName);
	switchField(MacroCell::Scratch0);	
      } loopClose();
      
      switchField(MacroCell::Scratch1);
      loopOpen(); { // else: pop frame
	zeroCell();
	popFrame(); // This leaves us at the Scratch1 cell in another frame: guaranteed 0
      } loopClose();
      switchField(MacroCell::Value0);

      API_EXPECT_NEXT("endBlock");
    } endBlock();
  }

  _currentFunction = nullptr;
  assert(_currentBlock == nullptr);
}
