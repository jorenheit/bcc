#include "compiler.ih"

// Pointer Management
void Compiler::resetOrigin() {
  _dp.set(0, static_cast<MacroCell::Field>(0));
}


void Compiler::pushPtr() {
  _ptrStack.push(_dp.current());
}

void Compiler::popPtr() {
  moveTo(_ptrStack.top());
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
					  std::vector<values::RValue> const &args, API_CTX) {
  auto const getHandles = [&](){
    std::vector<types::TypeHandle> result;
    for (auto const &arg: args) {
      result.push_back(arg.type());
    }
    return result;
  };

  _deferredFunctionCallTypeChecks.emplace_back(FunctionCall{
      .API_CTX_NAME = API_FWD,
      .caller = caller,
      .callee = callee,
      .args = getHandles()
    });
}

void Compiler::functionCallTypeChecks() {
  assert(_currentFunction == nullptr);

  for (auto const &[API_CTX_NAME, caller, callee, args]: _deferredFunctionCallTypeChecks) {
    
    auto const &params = _program.function(callee).sig.params;
    API_REQUIRE(params.size() == args.size(),
		"invalid number of arguments in function-call to '", callee, "': "
		"expected ", params.size(), ", got ", args.size(), ".");
    for (size_t i = 0; i != args.size(); ++i) {
      API_REQUIRE_ASSIGNABLE(params[i].type, args[i]);
    }
  }

  _deferredFunctionCallTypeChecks.clear();
}
