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
  
  // // Low byte
  moveTo(FrameLayout::TargetBlock, MacroCell::Value0);
  copyField(FrameLayout::TargetBlock, MacroCell::Payload0);
  switchField(MacroCell::Payload0);
  subConst(_currentBlock->globalBlockIndex & 0xff);
  notValue();

  // High Byte
  moveTo(FrameLayout::TargetBlock, MacroCell::Value1);
  copyField(FrameLayout::TargetBlock, MacroCell::Payload1);
  switchField(MacroCell::Payload1);
  subConst((_currentBlock->globalBlockIndex >> 8) & 0xff);
  notValue();

  // Run State
  moveTo(FrameLayout::RunState, MacroCell::Value0);
  copyField(FrameLayout::RunState, MacroCell::Payload0);

  // AND results into Flag
  moveTo(FrameLayout::TargetBlock, MacroCell::Flag);
  setToValue(0);
  moveTo(FrameLayout::TargetBlock, MacroCell::Payload0);
  loopOpen(); {
    zeroCell();
    moveTo(FrameLayout::TargetBlock, MacroCell::Payload1);
    loopOpen(); {
      zeroCell();
      moveTo(FrameLayout::RunState, MacroCell::Payload0);
      loopOpen(); {
	zeroCell();
	moveTo(FrameLayout::TargetBlock, MacroCell::Flag);
	setToValue(1);
	moveTo(FrameLayout::RunState, MacroCell::Payload0);
      } loopClose();
      moveTo(FrameLayout::TargetBlock, MacroCell::Payload1);
    } loopClose();
    moveTo(FrameLayout::TargetBlock, MacroCell::Payload0);
  } loopClose();

  // Clear helper cells
  moveTo(FrameLayout::TargetBlock, MacroCell::Payload0); zeroCell();
  moveTo(FrameLayout::TargetBlock, MacroCell::Payload1); zeroCell();
  moveTo(FrameLayout::RunState, MacroCell::Payload0);    zeroCell();

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
  assert(_currentFunction == nullptr);
  assert(_currentBlock == nullptr);

  for (size_t idx = 0; idx != _metaBlocks.size(); ++idx) {
    MetaBlock const &m = _metaBlocks[idx];
    
    _currentFunction = &_program.function(m.caller);
    Function const *callee = &_program.function(m.callee);

    // If a return-variable was provided, check that its type matches the returntype of the callee
    if (not m.returnVar.empty()) {
      assert(callee->sig.returnType == local(m.returnVar).type);
    }
    
    beginBlock(m.name); {

      if (callee->sig.returnType == _ts.voidT()) fetchReturnData();
      else {
	// Get or create the return slot to copy the return-variable into
	Slot const &returnSlot = [&](){
	  if (not m.returnVar.empty()) return local(m.returnVar);
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
}
