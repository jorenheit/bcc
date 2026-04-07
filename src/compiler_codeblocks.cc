#include "compiler.ih"

void Compiler::blockOpen() {
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
			       Temps<2>::pack(FrameLayout::TargetBlock, MacroCell::Scratch0,
					      FrameLayout::TargetBlock, MacroCell::Scratch1)
			       );

  moveTo(FrameLayout::RunState, MacroCell::Value0);
  andConstructive(/* result = */ Cell{FrameLayout::RunState, MacroCell::Flag},
		  /* other  = */ Cell{FrameLayout::TargetBlock, MacroCell::Flag},
		  Temps<2>::pack(FrameLayout::RunState, MacroCell::Scratch0,
				 FrameLayout::RunState, MacroCell::Scratch1)
		  );

  // Clear the targetblock flag
  moveTo(FrameLayout::TargetBlock, MacroCell::Flag);
  zeroCell();

  // Open the block, conditional on the run-flag
  moveTo(FrameLayout::RunState, MacroCell::Flag);
  loopOpen("block open");
  zeroCell();
  moveToOrigin();
}

void Compiler::blockClose() {
  assert(_currentBlock != nullptr);

  // We move back to the Flag stored in the RunState cell (guaranteed zero)
  moveTo(FrameLayout::RunState, MacroCell::Flag);
  loopClose("block close");
  moveToOrigin();
}

void Compiler::constructMetaBlocks() {
  assert(_currentFunction == nullptr);
  assert(_currentBlock == nullptr);

  for (size_t idx = 0; idx != _metaBlocks.size(); ++idx) {
    MetaBlock const &m = _metaBlocks[idx];
    Function const *callee = &_program.function(m.callee);

    // If a return-variable was provided, check that its type matches the returntype of the callee
    if (m.returnSlot) assert(callee->sig.returnType == m.returnSlot->type);

    // Set current function to caller (owner of metablock) and construct block
    _currentFunction = &_program.function(m.caller);    
    beginBlock(m.name); {

      if (callee->sig.returnType == TypeSystem::voidT()){
	fetchReturnData();
      }
      else {
	// Get or create the return slot to copy the return-variable into
	Slot const &returnSlot = [&](){
	  if (m.returnSlot) return *m.returnSlot;
	  static int retVarID = 0;
	  std::string const retVarName = std::string("__return_var_") + std::to_string(retVarID++);
	  return declareLocal(retVarName, callee->sig.returnType);
	}();

	fetchReturnData(returnSlot);

	// If we just wrote to a global, immediately sync this
	if (returnSlot.kind == Slot::GlobalReference) {
	  syncGlobal<&Compiler::putGlobal>(returnSlot);
	}
      }

      // Check if the run-state has become 0. If so, unwind the stack
      moveTo(FrameLayout::RunState, MacroCell::Value0);
      copyField(Cell{FrameLayout::RunState, MacroCell::Scratch0},
		Temps<1>::pack(_dp.current().offset, MacroCell::Scratch1)); 
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
      
    } endBlock();
  }

  _currentFunction = nullptr;
  assert(_currentBlock == nullptr);
}
