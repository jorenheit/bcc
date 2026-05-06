#include "assembler.ih"

void Assembler::setTargetSequence(primitive::Sequence *seq) {
  _currentSeq = seq;
}

std::string Assembler::builtinFunctionName(BuiltinFunction func) {
  switch (func) {
  case BuiltinFunction::PrintUnsigned8:   return "__print_i8";
  case BuiltinFunction::PrintUnsigned16:  return "__print_i16";
  case BuiltinFunction::PrintSigned8:     return "__print_s8";
  case BuiltinFunction::PrintSigned16:    return "__print_s16";
  }
  std::unreachable();
}

void Assembler::constructBuiltinFunctions() {
  assert(_currentFunction == nullptr);
  assert(_currentBlock == nullptr);

  for (auto func: _usedBuiltinFunctions) {

    types::TypeHandle const paramType = [&] {
      switch (func) {
      case BuiltinFunction::PrintUnsigned8:  return ts::i8();
      case BuiltinFunction::PrintUnsigned16: return ts::i16();
      case BuiltinFunction::PrintSigned8:    return ts::s8();
      case BuiltinFunction::PrintSigned16:   return ts::s16();
      }
      std::unreachable();
    }();

    std::string const funcName = builtinFunctionName(func);
    
    function(funcName).param("x", paramType).begin(); {
      if (types::isSignedInteger(paramType)) {
	printSignedImpl(Expression{ local("x") });
      } else {
	printUnsignedImpl(Expression{ local("x") });
      }
      returnFromFunction();
    } endFunction();
    
  }

  _usedBuiltinFunctions.clear();
}

primitive::Context Assembler::constructContext() const {

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

primitive::Sequence Assembler::compilePrimitives() const {

  primitive::Sequence result = _program.bootstrap;
  for (auto const &fn: _program.functions) {
    for (auto const &block:  fn.blocks) {
      result.append(block->code);
    }
  }
  result.append(_program.hatstrap);
  return result;
}

std::string Assembler::brainfuck(std::string const &name) const {
  // TODO: API_REQUIRE
  return simplifyBrainfuck(_bf.at(name));
}

std::string Assembler::primitives(std::string const &name) const {
  // TODO: API_REQUIRE
  return _txt.at(name);
}



primitive::Sequence Assembler::simplifySequence(primitive::Sequence const &seq) {
  primitive::Sequence result;
  if (seq.nodes.empty()) return result;

  result.nodes.push_back(seq.nodes[0]);
  for (size_t next = 1; next != seq.nodes.size(); ++next) {
    auto n0 = result.nodes.back();
    auto n1 = seq.nodes[next];

    if (auto merged = n0->merge(n1.get())) {
      result.nodes.back() = merged;
    }
    else {
      result.nodes.push_back(n1);
    }
  }
  return result;
}


// Compress BF string by cancelling opposite commands
std::string Assembler::simplifyBrainfuck(std::string const &bf) {
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
