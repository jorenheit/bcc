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
      assert(f.blocks.size() > 0 && "function does not contain any code-blocks."); // TODO: Error?
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

// Compress BF string by cancelling opposite commands
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
