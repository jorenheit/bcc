#include "compiler.ih"

// Pointer Management
void Compiler::resetOrigin() {
  assert(_dp.isStatic());
  _dp.resetTo(0);
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

void Compiler::deferFunctionCallTypeCheck(std::string const &caller,
					  std::string const &callee,
					  std::vector<Function::Arg> const &args) {
  _deferredFunctionCallTypeChecks.emplace_back(FunctionCall{
      .caller = caller,
      .callee = callee,
      .args = args
    });
}

void Compiler::functionCallTypeChecks() {
  assert(_currentFunction == nullptr);

  for (auto const &[caller, callee, args]: _deferredFunctionCallTypeChecks) {
    auto const &params = _program.function(callee).sig.params;
    assert(params.size() == args.size());

    _currentFunction = &_program.function(caller);
    for (size_t i = 0; i != args.size(); ++i) {
      if (args[i].kind == Function::Arg::Constant) {
	assert(params[i].type->isInteger());
      }
      else {
	Slot const &varSlot = local(args[i].varName);
	assert(params[i].type == varSlot.type);
      }
    }
    _currentFunction = nullptr;
  }
}
