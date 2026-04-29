
// FunctionCallBuilder templates
Assembler::FunctionCallBuilder &  Assembler::FunctionCallBuilder::into(auto&& result) & {
  _result = _assembler.lValue(std::forward<decltype(result)>(result), API_FWD);
  return *this;
}

Assembler::FunctionCallBuilder &&  Assembler::FunctionCallBuilder::into(auto&& result) && {
  return std::move(this->into(std::forward<decltype(result)>(result)));
}

Assembler::FunctionCallBuilder & Assembler::FunctionCallBuilder::arg(auto&& arg) & {
  _args.push_back(_assembler.rValue(std::forward<decltype(arg)>(arg), API_FWD));
  return *this;
}

Assembler::FunctionCallBuilder &&  Assembler::FunctionCallBuilder::arg(auto&& arg) && {
  return std::move(this->arg(std::forward<decltype(arg)>(arg)));
}
Assembler::FunctionCallBuilder::FunctionCallBuilder(Assembler &a, auto const &function, std::string const &nextBlockName, api::impl::Context const &ctx):
  BuilderBase("FunctionCallBuilder", "done", ctx),
  _assembler(a),
  _function(function),
  _nextBlockName(nextBlockName)
{}

