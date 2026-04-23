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


template <typename SpecType>
ExpressionResult Compiler::opAssignImpl(ExpressionResult const &lhs, ExpressionResult const &rhs, SpecType const &spec, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_BINOP(spec.op, lhs.type(), rhs.type());
  assert(not lhs.isLiteral());

  pushPtr();

  auto const [lhsBase, targetSlot, stride] = [&]() -> std::tuple<Slot, Slot, int> {
    Slot const lhsBase = lhs.slot()->materialize(*this);
    if (not types::isPointer(lhs.type())) {
      return {lhsBase, lhsBase, 1};
    }
    auto ptrType = types::cast<types::PointerType>(lhs.type());      
    return {
      lhsBase,
      lhsBase.sub(TypeSystem::i16(), RuntimePointer::Offset),
      ptrType->pointeeType()->size()
    };
  }();

  if (rhs.hasSlot()) {

    auto operandSlot = [&]() -> Slot {
      Slot const rhsSlot = rhs.slot()->materialize(*this);
      if (stride == 1) return rhsSlot;

      // Scale the operand
      Slot const opSlot = [&]() {
	if (not rhs.slot()->direct()) return rhsSlot;
	Slot const copy = getTemp(rhs.type());
	assignSlot(copy, rhsSlot);
	return copy;
      }();
            
      mulSlotByConst(opSlot, stride);
      return opSlot;
    }();
    
    (this->*spec.applyWithSlot)(targetSlot, operandSlot);
  }
  else {
    int const delta = values::cast<types::IntegerType>(rhs.literal())->value();
    (this->*spec.applyWithConst)(targetSlot, stride * delta);
  }

  if (not lhs.slot()->direct()) {
    lhs.slot()->write(*this, lhsBase);
  }
  
  popPtr();

  return lhs;
}

template <typename SpecType>
ExpressionResult Compiler::opImpl(ExpressionResult const &lhs, ExpressionResult const &rhs, SpecType const &spec, API_CTX) {
  API_CHECK_EXPECTED();
  API_REQUIRE_INSIDE_CODE_BLOCK();
  API_REQUIRE_BINOP(spec.op, lhs.type(), rhs.type());

  auto opResult = types::rules::binOpResult(spec.op, lhs.type(), rhs.type());
  assert(opResult);

  if (lhs.isLiteral() && rhs.isLiteral()) {
    int const x = values::cast<types::IntegerType>(lhs.literal())->value();
    int const y = values::cast<types::IntegerType>(rhs.literal())->value();
    auto const result = spec.fold(x, y);
    if (types::isI8(opResult.type))  return ExpressionResult{values::i8(result)};
    if (types::isI16(opResult.type)) return ExpressionResult{values::i16(result)};
    std::unreachable();
  }

  Slot result = getTemp(lhs.type());
  assignImpl(ExpressionResult{result}, lhs, API_FWD);
  opAssignImpl(ExpressionResult{result}, rhs, spec, API_FWD);

  result.type = opResult.type;
  return ExpressionResult{result};
}

#undef BINOP
