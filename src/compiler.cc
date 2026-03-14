#include <cassert>
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
#include <utility>
#include <cstdint>

#include "data.h"
#include "compiler.h"
#include "primitive.h"

void Compiler::begin() {
  assert(not _program.entryFunctionName.empty()); 
}

void Compiler::end() {
  // Done compiling the program. Generate the metablocks, bootstrap and hatstrap sequences.
  constructMetaBlocks();

  
  // To bootstrap the system, we need to do the following:
  // 1. Mark cell 0 using the FrameID field to indicate that this is where
  //    the global data frame starts. Leave the pointer in the value-field.
  // 2. Move the pointer to the first stackframe and reset the origin.
  // 3. Initialize the first stack-frame, where we set the TargetBlock to the
  //    index of the entry-function and the run-state to 1.
  // 4. Open the main-loop and leave the pointer in cell 0 of the frame.

  setTargetSequence(&_program.bootstrap);

  resetOrigin();
  _dp.setField(static_cast<MacroCell::Field>(0));
  switchField(MacroCell::FrameID);
  addConst(FrameLayout::GlobalVariableFrameID);
  switchField(MacroCell::Value0);

  moveTo(1 + _program.globalVariableFrameSize()); 

  resetOrigin();
  switchField(MacroCell::FrameID);
  addConst(FrameLayout::FirstStackFrameID);
  switchField(MacroCell::Value0);

  setNextBlock(_program.entryFunctionName); // [-]
  setToValue(1, FrameLayout::RunState); // >>>>>[-]+

  
  loopOpen(FrameLayout::RunState, "main loop"); // [
  moveToOrigin(); // <<<<<
  
  // Also generate the hatstrap code. All this needs to do is move the pointer to
  // the run-cell and close the loop.

  setTargetSequence(&_program.hatstrap);
  loopClose(FrameLayout::RunState, "main loop");
}

void Compiler::resetOrigin() {
  assert(_dp.isStatic());
  _dp.resetTo(0);
}

void Compiler::moveToOrigin() {
  moveTo(0);
}

void Compiler::pushPtr() {
  _ptrStack.push({
      .offset = _dp.staticOffset(),
      .field = _dp.activeField()
    });
}

void Compiler::popPtr() {
  auto [offset, field] = _ptrStack.top();
  moveTo(offset);
  switchField(field);
  _ptrStack.pop();
}

void Compiler::beginFunction(std::string name) {
  assert(_currentFunction == nullptr);
  _currentFunction = &_program.createFunction(std::move(name));
}

void Compiler::endFunction() {
  assert(_currentFunction != nullptr);
  assert(_currentBlock == nullptr);
  _currentFunction = nullptr;
}

void Compiler::setEntryPoint(std::string functionName) {
  _program.entryFunctionName = std::move(functionName);
}

void Compiler::beginBlock(std::string name) {
  assert(_currentFunction != nullptr);
  assert(_currentBlock == nullptr);

  auto globalIdx = _program.nextGlobalBlockIndex();
  Function::Block &block = _currentFunction->createBlock(std::move(name), globalIdx);
  _program.registerBlock(block);

  if (_currentFunction->blocks.size() == 1) {
    _currentFunction->entryBlockIndex = globalIdx;
  }

  _currentBlock = &block;
  _dp.resetTo(0);
  _nextBlockIsSet = false;

  setTargetSequence(&block.code);
  blockOpen();
}

void Compiler::endBlock() {
  assert(_currentBlock != nullptr);
  assert(_dp.isStatic());
  assert(_nextBlockIsSet);
      
  blockClose();
  _currentBlock = nullptr;
}


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


  auto const clearFieldIfActiveFieldNonzero = [&](MacroCell::Field targetField, int targetOffset = -1) {
    MacroCell::Field currentField = _dp.activeField();
    int currentOffset = _dp.staticOffset();
    
    loopOpen("Clear if nonzero"); {
      zeroCell();
      switchField(targetField);
      moveTo(targetOffset);
      zeroCell();
      moveTo(currentOffset);
      switchField(currentField);
    }; loopClose("Clear if nonzero");
  };


  // Step 1: Set-up the flag. We set it to 1 and reset it to 0 if either of the
  //         conditions fail.
  moveTo(FrameLayout::TargetBlock);
  switchField(MacroCell::Flag);
  setToValue(1);

  // Step 2: Compare the TargetBlock value with the index of this block.
  // a. Copy the value into the Runtime0 cell, using Runtime1 as a temporary scratch pad.
  // b. Subtract the index from the copy in RT0
  // c. Branch on the result -> if nonzero, reset the flag

  switchField(MacroCell::Value0);
  emit<primitive::CopyData>(MacroCell::Value0, MacroCell::Runtime0, MacroCell::Runtime1); 
  switchField(MacroCell::Runtime0);
  subConst(_currentBlock->globalBlockIndex);
  clearFieldIfActiveFieldNonzero(MacroCell::Flag);

  // Step 3: Check the Run-state
  // a. After moving to the Run-cell, copy the run-state into its Runtime0 field
  // b. Set its RT1 field to 1; this will be reset if run == 1 (effectively computing !run)
  // c. Use the value in RT1 (!Run) to reset the Enter-flag: if (!run) clear

  switchField(MacroCell::Value0);
  moveTo(FrameLayout::RunState);
  emit<primitive::CopyData>(MacroCell::Value0, MacroCell::Runtime0, MacroCell::Runtime1);

  switchField(MacroCell::Runtime1);
  setToValue(1);
  switchField(MacroCell::Runtime0);
  clearFieldIfActiveFieldNonzero(MacroCell::Runtime1);

  switchField(MacroCell::Runtime1);
  clearFieldIfActiveFieldNonzero(MacroCell::Flag, FrameLayout::TargetBlock);
  
  // Step 4: Enter block based on the Enter-flag in the TargetCell Block
  moveTo(FrameLayout::TargetBlock);
  switchField(MacroCell::Flag);
  loopOpen("block open");
  zeroCell();
  switchField(MacroCell::Value0);
}

void Compiler::blockClose() {
  assert(_dp.isStatic());
  assert(_currentBlock != nullptr);

  // We move back to the Flag stored in the TargetBlock cell
  moveTo(FrameLayout::TargetBlock);
  switchField(MacroCell::Flag);
  loopClose("block close");
  switchField(MacroCell::Value0);
  moveToOrigin();
}

void Compiler::loopOpen(std::string const &tag) {
  emit<primitive::LoopOpen>(tag);
}

void Compiler::loopOpen(int frameOffset, std::string const &tag) {
  moveTo(frameOffset);
  emit<primitive::LoopOpen>(tag);
}

void Compiler::loopClose(std::string const &tag) {
  emit<primitive::LoopClose>(tag);
}

void Compiler::loopClose(int frameOffset, std::string const &tag) {
  moveTo(frameOffset);
  emit<primitive::LoopClose>(tag);
}

void Compiler::pushFrame() {
  assert(_currentBlock != nullptr);
  assert(_currentFunction != nullptr);
  assert(_currentSeq != nullptr);


  pushPtr();
  
  // To push a frame, we need to move the pointer into the cell that marks the start of a fresh
  // frame, starting just beyond the current one. We also initialize the FrameID of this frame
  // with the current ID + 1.

  primitive::DInt currentFrameSize = [caller = _currentFunction->name](primitive::Context const &ctx){
    return ctx.getStackFrameSize(caller) * MacroCell::FieldCount;
  };
  
  moveToOrigin();
  switchField(MacroCell::FrameID);
  emit<primitive::CopyData>(MacroCell::FrameID,
			    currentFrameSize + MacroCell::FrameID,
			    currentFrameSize + MacroCell::Runtime0);
  emit<primitive::MovePointerRelative>(currentFrameSize);
  addConst(1);

  popPtr();
}

void Compiler::moveToPreviousFrame() {

  pushPtr();
  
  moveToOrigin();
  switchField(MacroCell::FrameID);
  emit<primitive::MovePointerDynamic>( /*Direction*/     primitive::Left,
				       /*Stride*/        MacroCell::FieldCount,
				       /*Until nonzero*/ MacroCell::FrameID,
				       /*Loop flag*/     MacroCell::Flag,
				       /*Scratch1*/      MacroCell::Runtime0,
				       /*Scratch2*/      MacroCell::Runtime1);
  popPtr();
}

void Compiler::popFrame() {
  assert(_currentBlock != nullptr);
  assert(_currentFunction != nullptr);
  assert(_currentSeq != nullptr);

  // Check if function is the entry-point of the program. If so, we need to set the run-cell to 0
  // If not, we do a dynamic move left until we hit the next frame marker.

  pushPtr();
  
  if (_currentFunction->name == _program.entryFunctionName) {
    zeroCell(FrameLayout::RunState);
    moveToOrigin();
  }
  else {
    moveToOrigin();
    switchField(MacroCell::FrameID);
    zeroCell();
    moveToPreviousFrame();
    // Pointer should now be at the start of the previous frame
  }
  
  popPtr();
}


void Compiler::fetchReturnData() {
  assert(_currentSeq != nullptr);  
  assert(_currentFunction != nullptr);

  // Need to fetch:
  // 1. Run-state
  // 2. Return-values (not implemented)

  primitive::DInt const forward = [caller = _currentFunction->name](primitive::Context const &ctx) -> int {
    return ctx.getStackFrameSize(caller) * MacroCell::FieldCount;
  };

  pushPtr();
  
  // Get run-state
  moveTo(FrameLayout::RunState);
  zeroCell();
  emit<primitive::MovePointerRelative>(forward);
  emit<primitive::MoveData>(-forward);
  emit<primitive::MovePointerRelative>(-forward);

  popPtr();
}
    
void Compiler::setNextBlock(int index) {
  assert(_currentSeq != nullptr);

  pushPtr();
  
  switchField(MacroCell::Value0);
  setToValue(index, FrameLayout::TargetBlock);
  
  popPtr();
  _nextBlockIsSet = true;
}
    
void Compiler::setNextBlock(std::string f, std::string b) {
  assert(_currentSeq != nullptr);

  // It is possible that the function or block name has not been
  // defined yet. So we need to check for this first.
  if (_program.isFunctionDefined(f)) {
    Function const &func = _program.function(f);
    if (func.isBlockDefined(b)) {
      // both defined -> use global block index
      return setNextBlock(func.block(b).globalBlockIndex);
    }
  }

  pushPtr();
  // Could not determine block index yet -> postpone until actual code generation
  zeroCell(FrameLayout::TargetBlock);
  emit<primitive::ChangeBy>([f, b](primitive::Context const &ctx) -> int {
    return ctx.getBlockIndex(f, b);
  });
  
  _nextBlockIsSet = true;
  popPtr();
}

    
Slot &Compiler::declareLocal(std::string const& name, int size) {
  assert(_currentFunction != nullptr);
  assert(!_currentFunction->frame.locals.contains(name));

  FrameLayout &frame = _currentFunction->frame;

  // First look for an existing slot that is empty at this point
  for (auto &[_, slot]: frame.locals) {
    if (slot.type == Slot::TempFree && size == slot.size) {
      slot.type = Slot::Local;
      slot.name = name;
      return slot;
    }
  }
  
  // No existing slot found that fits -> new slot
  Slot slot {
    .type = Slot::Local,
    .name = name,
    .offset = FrameLayout::LocalBase + frame.localAreaSize(),
    .size = size
  };
  auto [it, success] = frame.locals.emplace(name, slot);
  assert(success);
  return it->second;
}

Slot &Compiler::declareGlobal(std::string const &name, int size) {
  assert(_currentFunction == nullptr);
  assert(!_program.globals.contains(name));

  Slot slot {
    .type = Slot::Global,
    .name = name,
    .offset = _program.globalVariableFrameSize(),
    .size = size
  };
  auto [it, success] = _program.globals.emplace(name, slot);
  assert(success);
  return it->second;
}

void Compiler::referGlobals(std::vector<std::string> const &names) {
  assert(_currentFunction != nullptr);
  assert(_currentBlock == nullptr);

  beginBlock(std::string("__prologue_") + _currentFunction->name); {
    for (std::string const &name: names) {
      assert(_program.globals.contains(name));

      Slot const &globalSlot = _program.globals.at(name);
      Slot &localSlot = declareLocal(std::string("__g_") + name, globalSlot.size);
      localSlot.type = Slot::GlobalReference;
    }

    syncGlobalToLocal();
    setNextBlock(_program.nextGlobalBlockIndex());
  } endBlock();
}
    
Slot &Compiler::local(std::string const& name) {
  assert(_currentFunction != nullptr);
  auto &locals = _currentFunction->frame.locals;
  auto it = locals.find(name);
  if (it == locals.end()) {
    it = locals.find(std::string("__g_") + name);
  }
  assert(it != locals.end());
  return it->second;
}

Slot &Compiler::global(std::string const& name) {
  assert(_currentFunction != nullptr);
  auto &globals = _program.globals;
  auto it = globals.find(name);
  assert(it != globals.end());
  return it->second;
}
    
Slot &Compiler::allocateTemp(int size) {
  assert(_currentBlock != nullptr);

  Slot &slot = declareLocal("__tmp__", size);
  slot.type = Slot::TempUsed;
  return slot;
}

void Compiler::freeTemp(Slot &slot) {
  assert(_currentBlock != nullptr);
  assert(slot.type == Slot::TempUsed);
  slot.type = Slot::TempFree;
}

void Compiler::moveTo(int frameOffset) {
  assert(_currentSeq != nullptr);  
  assert(_dp.isStatic());
  if (frameOffset == -1) return;
  
  int const diff = frameOffset - _dp.staticOffset();
  if (diff == 0) return;

  emit<primitive::MovePointerRelative>(diff * MacroCell::FieldCount);
  _dp.moveRelative(diff);
}

void Compiler::switchField(MacroCell::Field field) {
  assert(_currentSeq != nullptr);
  
  auto current = static_cast<int>(_dp.activeField());
  auto target = static_cast<int>(field);
  if (current == target) return;
      
  emit<primitive::MovePointerRelative>(target - current);
  _dp.setField(field);
}


void Compiler::zeroCell(int frameOffset) { 
  moveTo(frameOffset);
  emit<primitive::ZeroCell>();
}

void Compiler::addConst(int delta, int frameOffset) {
  moveTo(frameOffset);
  emit<primitive::ChangeBy>(delta);
}

void Compiler::subConst(int delta, int frameOffset) {
  addConst(-delta, frameOffset);
}


void Compiler::setToValue(uint8_t value, int frameOffset) { 
  zeroCell(frameOffset);
  addConst(value, frameOffset);
}


void Compiler::writeOut(int frameOffset) {
  moveTo(frameOffset);
  emit<primitive::Out>();
}
    
void Compiler::callFunction(std::string const& functionName,
			    std::string const& nextBlockName) {
  assert(_currentFunction != nullptr);
  assert(_currentBlock != nullptr);
  assert(_currentSeq != nullptr);
  
  syncLocalToGlobal();

  auto const metaBlockName = std::string("__ret_meta_") 
    + _currentFunction->name + "_"
    + std::to_string(_metaBlocks.size());
  
  setNextBlock(_currentFunction->name, metaBlockName);
  _metaBlocks.push_back({
      .name = metaBlockName,
      .caller = _currentFunction->name,
      .nextBlockName = nextBlockName
    });

  pushFrame();
  setToValue(1, FrameLayout::RunState);
  setNextBlock(functionName);
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
      moveTo(FrameLayout::RunState);
      emit<primitive::CopyData>(MacroCell::Value0, MacroCell::Runtime0, MacroCell::Runtime1);
      switchField(MacroCell::Runtime1);
      setToValue(1);
      
      switchField(MacroCell::Runtime0);
      loopOpen(); { // if run: sync globals and set next block
	zeroCell();
	switchField(MacroCell::Runtime1);
	zeroCell();
	syncGlobalToLocal();
	setNextBlock(m.caller, m.nextBlockName);
	switchField(MacroCell::Runtime0);	
      } loopClose();
      
      switchField(MacroCell::Runtime1);
      loopOpen(); { // else: pop frame
	zeroCell();
	popFrame(); // This leaves us at the Runtime1 cell in another frame: guaranteed 0
      } loopClose();
      switchField(MacroCell::Value0);
      
    } endBlock();
  }

  _currentFunction = nullptr;
  assert(_currentBlock == nullptr);
  ran = true;
}

void Compiler::abortProgram() {
  assert(_currentBlock != nullptr);

  setToValue(0, FrameLayout::RunState);
  returnFromFunction();
}

void Compiler::returnFromFunction() {
  assert(_currentBlock != nullptr);
  
  // On return:
  // 1. Copy globals back. TODO: skip read-only globals
  // 2. Restore caller's frame (move pointer back until it hits the frame-marker)

  syncLocalToGlobal();
  popFrame();
  _nextBlockIsSet = true;
}

primitive::Context Compiler::constructContext() const {

  auto const constructBlockIDtoIndexMap = [&](){
    std::unordered_map<std::string, int> result;
    for (auto const &f: _program.functions) {
      // Special block ID equal to the first block of a function to indicate
      // the entry-point of this function (easy lookup).
      assert(f.blocks.size() > 0);
      result[f.name] = f.blocks[0]->globalBlockIndex;
      for (auto const &b: f.blocks) {
	std::string const id = f.name + "." + b->name;
	result[id] = b->globalBlockIndex;
      }
    }
    return result;
  };
  
  auto const constructStackFrameSizeMap = [&](){
    std::unordered_map<std::string, int> result;
    for (auto const &f: _program.functions) {
      result[f.name] = f.frame.totalLogicalCells();
    }
    return result;
  };
  
  return primitive::Context {
    .fieldCount = MacroCell::FieldCount,
    .blockIDtoIndex = constructBlockIDtoIndexMap(),
    .stackFrameSize = constructStackFrameSizeMap()
  };
}

primitive::Sequence Compiler::compilePrimitives() const {
  // TODO: assert that the program has been fully specified
  
  primitive::Sequence result = _program.bootstrap;
  for (auto const &fn: _program.functions) {
    for (auto const &block:  fn.blocks) {
      result.append(block->code);
    }
  }
  result.append(_program.hatstrap);
  return result;
}

std::string Compiler::dumpBrainfuck() const {
  primitive::Context ctx = constructContext();  
  return simplifyProgram(compilePrimitives().dumpCode(ctx));
}

std::string Compiler::dumpPrimitives() const {
  primitive::Context ctx = constructContext();  
  return compilePrimitives().dumpText(ctx);
}


void Compiler::syncGlobalToLocal() {
  return;
  
  auto const &locals = _currentFunction->frame.locals;
  auto const &globals = _program.globals;
      
  for (auto const &[localName, localSlot]: locals) {
    if (localSlot.type != Slot::GlobalReference) continue;
    std::string globalName = localName.substr(std::string("__g_").size());
    assert(globals.contains(globalName));

    Slot const &globalSlot = globals.at(globalName);
    assert(globalSlot.size == localSlot.size);

    // Move left dynamically until the FrameID equals the Global Marker
    moveToOrigin();
    switchField(MacroCell::Flag);
    setToValue(1);
    loopOpen(); {
      zeroCell();
      switchField(MacroCell::FrameID);
      moveToPreviousFrame();
      switchField(MacroCell::Flag);
      setToValue(1);
      
      // Copy the marker value into RT0
      switchField(MacroCell::Value0);
      emit<primitive::CopyData>(MacroCell::Value0, MacroCell::Runtime0, MacroCell::Runtime1);
    
      // Subtract the global marker value from the copy in RT0
      switchField(MacroCell::Runtime0);
      subConst(FrameLayout::GlobalVariableFrameID);

      // If nonzero, we're not here yet -> set enter to 0
      loopOpen("Clear if nonzero"); {
	zeroCell();
	switchField(MacroCell::Flag);
	zeroCell();
	switchField(MacroCell::Runtime0);
      }; loopClose("Clear if nonzero");

      switchField(MacroCell::Flag);
    } loopClose();

    
    
  }
  
}


void Compiler::syncLocalToGlobal() {
  // TODO: implement
}

std::string Compiler::simplifyProgram(std::string const &bf) {
  auto cancel = [](std::string const &input, char const up, char const down) -> std::string {
    std::string result;
    int count = 0;

    auto flush = [&]() {
      if (count > 0) result += std::string( count, up);
      if (count < 0) result += std::string(-count, down);
      count = 0;
    };

    for (char c: input) {
      if (c == up)   ++count;
      else if (c == down) --count;
      else {
	flush();
	result += c;
      }
    }
    
    flush();
    return result;
  };
  
  return cancel(cancel(bf, '>', '<'), '+', '-');
}
