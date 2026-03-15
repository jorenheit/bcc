#include "compiler.ih"

void Compiler::blockOpen() {
  assert(_dp.isStatic());
  assert(_dp.staticOffset() == 0);
  assert(_currentBlock != nullptr);

  // To start a block, we need to check 2 conditions:
  // 1. Does the block-index match the value stored in the TargetBlock cell?
  // 2. Is the Run-cell still set?
  //
  // If both are true, the Flag0 field of the TargetBlock cell is set to 1 and
  // used as the conditional cell upon which it is decided whether or not to enter
  // the block.


  // TODO: this can probably be replaced by a not-operation
  auto const clearFieldIfActiveFieldNonzero = [&](MacroCell::Field targetField, int targetOffset = -1) {
    MacroCell::Field currentField = _dp.activeField();
    int currentOffset = _dp.staticOffset();
    
    loopOpen(); {
      zeroCell();
      switchField(targetField);
      moveTo(targetOffset);
      zeroCell();
      moveTo(currentOffset);
      switchField(currentField);
    }; loopClose();
  };


  // Step 1: Set-up the flag. We set it to 1 and reset it to 0 if either of the
  //         conditions fail.
  moveTo(FrameLayout::TargetBlock);
  switchField(MacroCell::Flag0);
  setToValue(1);

  // Step 2: Compare the TargetBlock value with the index of this block.
  // a. Copy the value into the Scratch0 cell, using Scratch1 as a temporary scratch pad.
  // b. Subtract the index from the copy in RT0
  // c. Branch on the result -> if nonzero, reset the flag
  moveTo(FrameLayout::TargetBlock, MacroCell::Value0);
  copyField(FrameLayout::TargetBlock, MacroCell::Scratch0);
  
  switchField(MacroCell::Scratch0);
  subConst(_currentBlock->globalBlockIndex);
  clearFieldIfActiveFieldNonzero(MacroCell::Flag0);

  // Step 3: Check the Run-state
  // a. After moving to the Run-cell, copy the run-state into its Scratch0 field
  // b. Set its RT1 field to 1; this will be reset if run == 1 (effectively computing !run)
  // c. Use the value in RT1 (!Run) to reset the Enter-flag: if (!run) clear

  moveTo(FrameLayout::RunState, MacroCell::Value0);
  copyField(FrameLayout::RunState, MacroCell::Scratch0); 

  moveTo(FrameLayout::RunState, MacroCell::Scratch1);
  setToValue(1);
  switchField(MacroCell::Scratch0);
  clearFieldIfActiveFieldNonzero(MacroCell::Scratch1);

  switchField(MacroCell::Scratch1);
  clearFieldIfActiveFieldNonzero(MacroCell::Flag0, FrameLayout::TargetBlock);
  
  // Step 4: Enter block based on the flag in the TargetCell Block
  moveTo(FrameLayout::TargetBlock, MacroCell::Flag0);
  loopOpen("block open");
  zeroCell();
  switchField(MacroCell::Value0);
}

void Compiler::blockClose() {
  assert(_dp.isStatic());
  assert(_currentBlock != nullptr);

  // We move back to the Flag stored in the TargetBlock cell
  moveTo(FrameLayout::TargetBlock, MacroCell::Flag0);
  loopClose("block close");
  moveToOrigin();
  switchField(MacroCell::Value0);
}

void Compiler::constructMetaBlocks() {
  static bool ran = false;
  assert(!ran);
  
  assert(_currentFunction == nullptr);
  assert(_currentBlock == nullptr);

  for (size_t idx = 0; idx != _metaBlocks.size(); ++idx) {
    MetaBlock const &m = _metaBlocks[idx];
    
    _currentFunction = &_program.function(m.caller);
    beginBlock(m.name); {
      fetchReturnData();

      // Check if the run-state has become 0. If so, unwind the stack
      moveTo(FrameLayout::RunState, MacroCell::Value0);
      copyField(FrameLayout::RunState, MacroCell::Scratch0); 
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
  ran = true;
}
