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
					  std::vector<values::Value> const &args) {
  auto const getHandles = [&](){
    std::vector<types::TypeHandle> result;
    for (auto const &arg: args) {
      types::TypeHandle type = arg->type(_ts);
      if (type == nullptr) {
	type = local(arg->varName()).type;
      }
      result.push_back(type);
    }
    return result;
  };

  _deferredFunctionCallTypeChecks.emplace_back(FunctionCall{
      .caller = caller,
      .callee = callee,
      .args = getHandles()
    });
}

void Compiler::functionCallTypeChecks() {
  assert(_currentFunction == nullptr);

  for (auto const &[caller, callee, args]: _deferredFunctionCallTypeChecks) {
    auto const &params = _program.function(callee).sig.params;
    error_if(params.size() != args.size(),
	     "invalid number of arguments in call to '", callee, "' (in funcion '", caller, "'): ",
	     "expected ", params.size(), ", got ", args.size(), ".");
    for (size_t i = 0; i != args.size(); ++i) {
      error_if(args[i] != params[i].type,
	       "type mismatch in argument ", (i+1), " of call to '", callee, "' (in function '", caller, "'): ",
	       "expected '", params[i].type->str(), "', got '", args[i]->str(), "'.");
    }
  }
}
