// TODO: there must be a cleaner way to define a struct
template <typename ... Args>
StructFields Compiler::constructFields(Args&& ... args) {
  static_assert(sizeof ... (Args) % 2 == 0);
  StructFields fields; fields.reserve(sizeof...(args) / 2);

  auto addField = [&]<typename ... Rest>(auto&& self, std::string const &name, types::TypeHandle type, Rest&& ... rest) -> void {
    fields.push_back(StructField{name, type});
    if constexpr (sizeof ... (Rest) == 0) return;
    else self(self, std::forward<Rest>(rest)...);
  };

  addField(addField, std::forward<Args>(args)...);
  return fields;
}    

template <typename... Args>
Compiler::ArgList Compiler::constructFunctionArguments_(API_FUNC_SOURCE, Args&&... args) {
  API_FUNC_BEGIN("constructFunctionArguments");
  ArgList result;
  result.reserve(sizeof...(args));
  (result.emplace_back(rValue(std::forward<Args>(args), API_FWD)), ...);
  return result;
}

template <typename Ret>
void Compiler::callFunction(std::string const &functionName, std::string const &nextBlockName,
			    ArgList const &args, Ret const &returnSlot, API_FUNC_SOURCE) {
  API_FUNC_BEGIN("callFunction");
  callFunctionImpl(functionName, nextBlockName, lValue(returnSlot, API_FWD), args, API_FWD);
}

template <typename R>
void Compiler::returnFromFunction(R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("returnFromFunction");
  returnFromFunctionImpl(rValue(rhs, API_FWD), API_FWD);
}

template <typename L, typename R>
void Compiler::assign(L const &lhs, R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("assign");
  assignImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

template <typename R>
void Compiler::writeOut(R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("writeOut");
  writeOutImpl(rValue(rhs, API_FWD), API_FWD);
}

template <typename L>
SlotProxy Compiler::structField(L const &obj, std::string const &field, API_FUNC_SOURCE) { API_FUNC_BEGIN("structField");
  return structFieldImpl(lValue(obj, API_FWD), field, API_FWD);
}

template <typename L>
SlotProxy Compiler::structField(L const &obj, int fieldIndex, API_FUNC_SOURCE) { API_FUNC_BEGIN("structField");
  return structFieldImpl(lValue(obj, API_FWD), fieldIndex, API_FWD);
}

template <typename Array>
SlotProxy Compiler::arrayElement(Array const &arr, int index, API_FUNC_SOURCE) { API_FUNC_BEGIN("arrayElement");
  return arrayElementImpl(lValue(arr, API_FWD), index, API_FWD);
}

template <typename Array, typename Index>
SlotProxy Compiler::arrayElement(Array const &arr, Index const &index, API_FUNC_SOURCE) { API_FUNC_BEGIN("arrayElement");
  return arrayElementImpl(lValue(arr, API_FWD), rValue(index, API_FWD), API_FWD);
}

template <typename Pointer>
SlotProxy Compiler::dereferencePointer(Pointer const &ptr, API_FUNC_SOURCE) { API_FUNC_BEGIN("dereferencePointer");
  return dereferencePointerImpl(rValue(ptr, API_FWD), API_FWD);
}

// TODO: xxxAssign should take LValue, but xxx can take an RValue
template <typename L, typename R>
void Compiler::addAssign(L const &lhs, R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("addAssign");
  addAssignImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

template <typename L, typename R>
SlotProxy Compiler::add(L const &lhs, R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("add");
  return addImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

template <typename L, typename R>
void Compiler::subAssign(L const &lhs, R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("subAssign");
  subAssignImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

template <typename L, typename R>
SlotProxy Compiler::sub(L const &lhs, R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("sub");
  return subImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

template <typename L, typename R>
void Compiler::mulAssign(L const &lhs, R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("mulAssign");
  mulAssignImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

template <typename L, typename R>
SlotProxy Compiler::mul(L const &lhs, R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("mul");
  return mulImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

template <typename L, typename R>
void Compiler::divAssign(L const &lhs, R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("divAssign");
  divAssignImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

template <typename L, typename R>
SlotProxy Compiler::div(L const &lhs, R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("div");
  return divImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

template <typename L, typename R>
void Compiler::modAssign(L const &lhs, R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("divAssign");
  modAssignImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

template <typename L, typename R>
SlotProxy Compiler::mod(L const &lhs, R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("mod");
  return modImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

template <typename L, typename R>
SlotProxy Compiler::eq(L const &lhs, R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("eq");
  return eqImpl(rValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

template <typename L, typename R>
SlotProxy Compiler::neq(L const &lhs, R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("neq");
  return neqImpl(rValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

template <typename L, typename R>
SlotProxy Compiler::lt(L const &lhs, R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("lt");
  return ltImpl(rValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

template <typename L, typename R>
SlotProxy Compiler::le(L const &lhs, R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("le");
  return leImpl(rValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

template <typename L, typename R>
SlotProxy Compiler::gt(L const &lhs, R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("gt");
  return gtImpl(rValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

template <typename L, typename R>
SlotProxy Compiler::ge(L const &lhs, R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("ge");
  return geImpl(rValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

template <typename L>
SlotProxy Compiler::addressOf(L const &obj, API_FUNC_SOURCE) { API_FUNC_BEGIN("addressOf");
  return addressOfImpl(lValue(obj, API_FWD), API_FWD);
}

template <typename Condition>
void Compiler::branchIf(Condition const &condition, std::string const &trueLabel,
	      std::string const &falseLabel, API_FUNC_SOURCE) { API_FUNC_BEGIN("branchIf");
  return branchIfImpl(rValue(condition, API_FWD), trueLabel, falseLabel, API_FWD);
}
