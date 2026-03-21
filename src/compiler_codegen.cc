#include "compiler.ih"

void Compiler::setTargetSequence(primitive::Sequence *seq) {
  _currentSeq = seq;
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

  auto const constructLocalBaseOffsetMap = [&](){
    std::unordered_map<std::string, int> result;
    for (auto const &f: _program.functions) {
      result[f.name] = f.frame.localBase();
    }
    return result;
  };
  
  return primitive::Context {
    .fieldCount = MacroCell::FieldCount,
    .blockIDtoIndex = constructBlockIDtoIndexMap(),
    .stackFrameSize = constructStackFrameSizeMap(),
    .localBaseOffset = constructLocalBaseOffsetMap()
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
