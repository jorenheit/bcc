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
#define constructFunctionArguments(...) constructFunctionArguments_(std::source_location::current(), __VA_ARGS__)

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

template <typename R>
void Compiler::writeOut(R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("writeOut");
  writeOutImpl(rValue(rhs, API_FWD), API_FWD);
}

template <typename L, typename R>
ExpressionResult Compiler::assign(L const &lhs, R const &rhs, API_FUNC_SOURCE) { API_FUNC_BEGIN("assign");
  return assignImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
}

template <typename L>
ExpressionResult Compiler::structField(L const &obj, std::string const &field, API_FUNC_SOURCE) { API_FUNC_BEGIN("structField");
  return structFieldImpl(rValue(obj, API_FWD), field, API_FWD);
}

template <typename L>
ExpressionResult Compiler::structField(L const &obj, int fieldIndex, API_FUNC_SOURCE) { API_FUNC_BEGIN("structField");
  return structFieldImpl(rValue(obj, API_FWD), fieldIndex, API_FWD);
}

template <typename Array>
ExpressionResult Compiler::arrayElement(Array const &arr, int index, API_FUNC_SOURCE) { API_FUNC_BEGIN("arrayElement");
  return arrayElementImpl(rValue(arr, API_FWD), index, API_FWD);
}

template <typename Array, typename Index>
ExpressionResult Compiler::arrayElement(Array const &arr, Index const &index, API_FUNC_SOURCE) { API_FUNC_BEGIN("arrayElement");
  return arrayElementImpl(rValue(arr, API_FWD), rValue(index, API_FWD), API_FWD);
}

template <typename Pointer>
ExpressionResult Compiler::dereferencePointer(Pointer const &ptr, API_FUNC_SOURCE) { API_FUNC_BEGIN("dereferencePointer");
  return dereferencePointerImpl(lValue(ptr, API_FWD), API_FWD);
}

template <typename L>
ExpressionResult Compiler::addressOf(L const &obj, API_FUNC_SOURCE) { API_FUNC_BEGIN("addressOf");
  return addressOfImpl(lValue(obj, API_FWD), API_FWD);
}

template <typename Condition>
void Compiler::branchIf(Condition const &condition, std::string const &trueLabel,
	      std::string const &falseLabel, API_FUNC_SOURCE) { API_FUNC_BEGIN("branchIf");
  return branchIfImpl(rValue(condition, API_FWD), trueLabel, falseLabel, API_FWD);
}

// Binary operations
// The implementation for opImpl and opAssignImpl is in compiler_binop_general.cc

#define BINOP(OP)							\
  template <typename L, typename R>					\
  ExpressionResult Compiler::OP##Assign(L const &lhs, R const &rhs, API_FUNC_SOURCE) { \
    API_FUNC_BEGIN(#OP "Assign");					\
    return opAssignImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), OP##Spec, API_FWD); \
  }									\
									\
  template <typename L, typename R>					\
  ExpressionResult Compiler::OP(L const &lhs, R const &rhs, API_FUNC_SOURCE) { \
    API_FUNC_BEGIN(#OP);						\
    return opImpl(rValue(lhs, API_FWD), rValue(rhs, API_FWD), OP##Spec, API_FWD); \
  }

// Arithmetic
BINOP(add);
BINOP(sub);
BINOP(mul);
BINOP(div);
BINOP(mod);

// Logic
BINOP(land);
BINOP(lnand);
BINOP(lor);
BINOP(lnor);
BINOP(lxor);
BINOP(lxnor);

// Comparisons
BINOP(eq);
BINOP(neq);
BINOP(lt);
BINOP(le);
BINOP(gt);
BINOP(ge);

#undef BINOP

