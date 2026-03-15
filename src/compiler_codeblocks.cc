#include "compiler.ih"

void Compiler::blockOpen() {
  assert(_dp.isStatic());
  assert(_dp.staticOffset() == 0);
  assert(_currentBlock != nullptr);

  // To start a block, we need to check 2 conditions:
  // 1. Does the block-index match the value stored in the TargetBlock cell?
  // 2. Is the Run-cell still set?
  //
  // If both are true, the Flag field of the TargetBlock cell is set to 1 and
  // used as the conditional cell upon which it is decided whether or not to enter
  // the block.

  // Condition 1: Compare the TargetBlock value with the index of this block.
  // a. Copy the value into the Flag cell
  // b. Subtract the index from the copy
  // c. Compute NOT on the flag
  
  moveTo(FrameLayout::TargetBlock, MacroCell::Value0);
  copyField(FrameLayout::TargetBlock, MacroCell::Flag);
  switchField(MacroCell::Flag);
  subConst(_currentBlock->globalBlockIndex);
  notValue();

  // Condition 2: If TargetBlock::Flag was set, we need to check the RunState
  // a. Copy the current TargetBlock::Flag into the RunState::Flag
  // b. If set, compute the NOT on the RunState
  // c. If NOT(run), reset the TargetBlock::Flag
  
  copyField(FrameLayout::RunState, MacroCell::Flag);
  moveTo(FrameLayout::RunState, MacroCell::Flag);
  loopOpen(); {
    zeroCell();
    
    moveTo(FrameLayout::RunState, MacroCell::Value0);
    notValue(MacroCell::Flag);
    moveTo(FrameLayout::RunState, MacroCell::Flag);
    loopOpen(); {
      zeroCell();
      moveTo(FrameLayout::TargetBlock, MacroCell::Flag);
      zeroCell();
      moveTo(FrameLayout::RunState, MacroCell::Flag);
    } loopClose();
    
    moveTo(FrameLayout::RunState, MacroCell::Flag);
  } loopClose();


  // Start Block, conditional on the computed flag (index match && run == 1)
  moveTo(FrameLayout::TargetBlock, MacroCell::Flag);
  loopOpen("block open");
  zeroCell();
  switchField(MacroCell::Value0);
}

void Compiler::blockClose() {
  assert(_dp.isStatic());
  assert(_currentBlock != nullptr);

  // We move back to the Flag stored in the TargetBlock cell
  moveTo(FrameLayout::TargetBlock, MacroCell::Flag);
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
