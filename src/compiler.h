#pragma once
#include <stack>
#include <sstream>
#include <optional>
#include <unordered_set>
#include "proxy.h"
#include "program.h"
#include "function.h"
#include "data.h"
#include "rlvalue.h"
#include "types.h"

#define API_HEADER
#include "api.h"

// ============================================================
// Compiler
// ============================================================

class Compiler {
  friend class proxy::Impl::Direct;
  friend class proxy::Impl::ArrayElement;
  friend class proxy::Impl::StructField;
  friend class proxy::Impl::DereferencedPointer;
  friend class api::Context;
  
  Program _program;
  Function* _currentFunction = nullptr;
  Function::Block* _currentBlock = nullptr;
  Function::Scope* _currentScope = nullptr;
  primitive::Sequence* _currentSeq = nullptr; 
  bool _nextBlockIsSet = false;

  struct {
    bool begun = false;
    bool allowGlobalDefinitions = true; // TODO: rename to allowGlobalDeclarations
  } _state;
  
  DataPointer _dp;

  struct MetaBlock {
    std::string name;
    std::string caller;
    std::string callee;
    std::optional<SlotProxy> returnSlot;
    std::string nextBlockName;
  };
  std::vector<MetaBlock> _metaBlocks;

  std::stack<Cell> _ptrStack;

  struct FunctionCall {
    api::Context API_CTX_NAME;
    std::string caller, callee;
    std::vector<types::TypeHandle> args;
  };
  std::vector<FunctionCall> _deferredFunctionCallTypeChecks;

  std::unordered_set<std::string> _aliasedGlobals;
  
public:
  inline Compiler() { TypeSystem::init(); }

  // TODO: API_FUNC 
  std::string dumpPrimitives() const;
  std::string dumpBrainfuck() const;

  // IR Directives
  // TODO: move implementation to compiler_public.cc and compiler_public.tpp
  void setEntryPoint(std::string functionName, API_FUNC);
  void begin(API_FUNC);
  void end(API_FUNC);
  void endFunction(API_FUNC);
  void beginScope(API_FUNC);
  void endScope(API_FUNC);
  void beginBlock(std::string name, API_FUNC);
  void endBlock(API_FUNC);
  void setNextBlock(int index, API_FUNC);
  void setNextBlock(std::string const &b, API_FUNC);
  void setNextBlock(std::string const &f, std::string const &b, API_FUNC);
  void abortProgram(API_FUNC);
  void referGlobals(std::vector<std::string> const &names, API_FUNC);


  // TODO: there must be a cleaner way to define a struct
  template <typename ... Args>
  StructFields constructFields(Args&& ... args) {
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

  types::TypeHandle defineStruct(std::string const& name, StructFields const &fields, API_FUNC);

  // TODO: there must be a cleaner way to pass args
  using ArgList = std::vector<values::RValue>;

  template <typename... Args>
  ArgList constructFunctionArguments_(API_FUNC_SOURCE, Args&&... args) {
    API_FUNC_BEGIN("constructFunctionArguments");
    ArgList result;
    result.reserve(sizeof...(args));
    (result.emplace_back(rValue(std::forward<Args>(args), API_FWD)), ...);
    return result;
  }
#define constructFunctionArguments(...) constructFunctionArguments_(std::source_location::current(), __VA_ARGS__)
  
  void callFunction(std::string const& functionName, std::string const& nextBlockName, API_FUNC);  
  void callFunction(std::string const& functionName, std::string const& nextBlockName, ArgList const &args, API_FUNC);
  
  template <typename L>
  void callFunction(std::string const& functionName, std::string const& nextBlockName,
		    ArgList const &args, L&& returnSlot, API_FUNC) {
    API_FUNC_BEGIN("callFunction");
    callFunctionImpl(functionName, nextBlockName, lValue(std::forward<L>(returnSlot), API_FWD), args, API_FWD);
  }

  void returnFromFunction(API_FUNC) { API_FUNC_BEGIN("returnFromFunction"); returnFromFunctionImpl({}, API_FWD); }

  // TODO: seperate template declarations from implementations in tpp file
  template <typename R>
  void returnFromFunction(R const &rhs, API_FUNC) { API_FUNC_BEGIN("returnFromFunction");
    returnFromFunctionImpl(rValue(rhs, API_FWD), API_FWD);
  }

  template <typename L, typename R>
  void assign(L const &lhs, R const &rhs, API_FUNC) { API_FUNC_BEGIN("assign");
    assignImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
  }

  template <typename R>
  void writeOut(R const &rhs, API_FUNC) { API_FUNC_BEGIN("writeOut");
    writeOutImpl(rValue(rhs, API_FWD), API_FWD);
  }

  template <typename L>
  SlotProxy structField(L const &obj, std::string const &field, API_FUNC) { API_FUNC_BEGIN("structField");
    return structFieldImpl(lValue(obj, API_FWD), field, API_FWD);
  }

  template <typename L>
  SlotProxy structField(L const &obj, int fieldIndex, API_FUNC) { API_FUNC_BEGIN("structField");
    return structFieldImpl(lValue(obj, API_FWD), fieldIndex, API_FWD);
  }

  template <typename Array>
  SlotProxy arrayElement(Array const &arr, int index, API_FUNC) { API_FUNC_BEGIN("arrayElement");
    return arrayElementImpl(lValue(arr, API_FWD), index, API_FWD);
  }
  
  template <typename Array, typename Index>
  SlotProxy arrayElement(Array const &arr, Index const &index, API_FUNC) { API_FUNC_BEGIN("arrayElement");
    return arrayElementImpl(lValue(arr, API_FWD), rValue(index, API_FWD), API_FWD);
  }

  template <typename Pointer>
  SlotProxy dereferencePointer(Pointer const &ptr, API_FUNC) { API_FUNC_BEGIN("dereferencePointer");
    return dereferencePointerImpl(rValue(ptr, API_FWD), API_FWD);
  }

  // TODO: xxxAssign should take LValue, but xxx can take an RValue
  template <typename L, typename R>
  void addAssign(L const &lhs, R const &rhs, API_FUNC) { API_FUNC_BEGIN("addAssign");
    addAssignImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
  }

  template <typename L, typename R>
  SlotProxy add(L const &lhs, R const &rhs, API_FUNC) { API_FUNC_BEGIN("add");
    return addImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
  }

  template <typename L, typename R>
  void subAssign(L const &lhs, R const &rhs, API_FUNC) { API_FUNC_BEGIN("subAssign");
    subAssignImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
  }

  template <typename L, typename R>
  SlotProxy sub(L const &lhs, R const &rhs, API_FUNC) { API_FUNC_BEGIN("sub");
    return subImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
  }
  
  template <typename L, typename R>
  void mulAssign(L const &lhs, R const &rhs, API_FUNC) { API_FUNC_BEGIN("mulAssign");
    mulAssignImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
  }

  template <typename L, typename R>
  SlotProxy mul(L const &lhs, R const &rhs, API_FUNC) { API_FUNC_BEGIN("mul");
    return mulImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
  }


  template <typename L, typename R>
  void divAssign(L const &lhs, R const &rhs, API_FUNC) { API_FUNC_BEGIN("divAssign");
    divAssignImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
  }
  
  template <typename L, typename R>
  SlotProxy div(L const &lhs, R const &rhs, API_FUNC) { API_FUNC_BEGIN("div");
    return divImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
  }

  template <typename L, typename R>
  void modAssign(L const &lhs, R const &rhs, API_FUNC) { API_FUNC_BEGIN("divAssign");
    modAssignImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
  }
  
  template <typename L, typename R>
  SlotProxy mod(L const &lhs, R const &rhs, API_FUNC) { API_FUNC_BEGIN("mod");
    return modImpl(lValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
  }
  
  template <typename L, typename R>
  SlotProxy eq(L const &lhs, R const &rhs, API_FUNC) { API_FUNC_BEGIN("eq");
    return eqImpl(rValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
  }

  template <typename L, typename R>
  SlotProxy neq(L const &lhs, R const &rhs, API_FUNC) { API_FUNC_BEGIN("neq");
    return neqImpl(rValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
  }
  
  template <typename L, typename R>
  SlotProxy lt(L const &lhs, R const &rhs, API_FUNC) { API_FUNC_BEGIN("lt");
    return ltImpl(rValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
  }

  template <typename L, typename R>
  SlotProxy le(L const &lhs, R const &rhs, API_FUNC) { API_FUNC_BEGIN("le");
    return leImpl(rValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
  }

  template <typename L, typename R>
  SlotProxy gt(L const &lhs, R const &rhs, API_FUNC) { API_FUNC_BEGIN("gt");
    return gtImpl(rValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
  }

  template <typename L, typename R>
  SlotProxy ge(L const &lhs, R const &rhs, API_FUNC) { API_FUNC_BEGIN("ge");
    return geImpl(rValue(lhs, API_FWD), rValue(rhs, API_FWD), API_FWD);
  }
  
  
  
  template <typename L>
  SlotProxy addressOf(L const &obj, API_FUNC) { API_FUNC_BEGIN("addressOf");
    return addressOfImpl(lValue(obj, API_FWD), API_FWD);
  }

  template <typename Condition>
  void branchIf(Condition const &condition, std::string const &trueLabel,
		std::string const &falseLabel, API_FUNC) { API_FUNC_BEGIN("branchIf");
    return branchIfImpl(rValue(condition, API_FWD), trueLabel, falseLabel, API_FWD);
  }

  
  Slot declareLocal(std::string const &name, types::TypeHandle type, API_FUNC);
  Slot declareGlobal(std::string const &name, types::TypeHandle type, API_FUNC);
  Slot declareGlobalReference(Slot const &globalSlot);

  // TODO: rename to resolveVar(...)
  Slot local(std::string const& name, bool globalReference = false) const;


  // TODO: replace FunctionSignature with types::FunctionType and accept a vector of strings that
  //       bind variables to the param-types.

  template <typename ... Args>
  FunctionSignature constructFunctionSignature(Args&& ... args) {
    return FunctionSignature{std::forward<Args>(args)...};
  }
  
  void beginFunction(std::string const &name, FunctionSignature const &sig, API_FUNC); 
  inline void beginFunction(std::string const &name) {
    beginFunction(name, constructFunctionSignature(TypeSystem::voidT()));
  }

  
private:
  // Diagnostics (compiler_diag.cc)
  std::string currentFunction() const;
  std::string currentBlock() const;
  bool programStarted() const;
  bool declaredAsGlobal(std::string const &name) const;
  bool globalDeclarationsAllowed() const;
  bool inScope(std::string const &name) const;
  bool inCurrentScope(std::string const &name) const;
  int currentScopeDepth() const;
  
  // Normalize to RValue or LValue (compiler_rlvalue.cc)
  values::RValue rValue(values::RValue const &val, API_CTX) const;
  values::RValue rValue(std::string const &var, API_CTX) const;
  values::RValue rValue(SlotProxy const &slot, API_CTX) const;
  values::RValue rValue(values::Literal const &val, API_CTX) const;
  values::LValue lValue(values::LValue const &val, API_CTX) const;
  values::LValue lValue(std::string const &var, API_CTX) const;  
  values::LValue lValue(SlotProxy const &slot, API_CTX) const;

  // Implementation functions for public interface
  void setNextBlockImpl(int index);
  void setNextBlockImpl(std::string const &f, std::string const &b);
  
  void callFunctionImpl(std::string const& functionName, std::string const& nextBlockName,
			std::optional<values::LValue> const &returnSlot, ArgList const &args, API_CTX);
  void returnFromFunctionImpl(std::optional<values::RValue> const &ret, API_CTX);
  SlotProxy structFieldImpl(values::LValue const &obj, std::string const &field, API_CTX);
  SlotProxy structFieldImpl(values::LValue const &obj, int fieldIndex, API_CTX);
  SlotProxy arrayElementImpl(values::LValue const &arr, int index, API_CTX);
  SlotProxy arrayElementImpl(values::LValue const &arr, values::RValue const &index, API_CTX);
  SlotProxy dereferencePointerImpl(values::RValue const &ptr, API_CTX);

  // Arithmetic operators
  SlotProxy addImpl(values::LValue const &lhs, values::RValue const &rhs, API_CTX);
  SlotProxy subImpl(values::LValue const &lhs, values::RValue const &rhs, API_CTX);
  SlotProxy mulImpl(values::LValue const &lhs, values::RValue const &rhs, API_CTX);
  SlotProxy divImpl(values::LValue const &lhs, values::RValue const &rhs, API_CTX);
  SlotProxy modImpl(values::LValue const &lhs, values::RValue const &rhs, API_CTX);

  // Comparison operators
  SlotProxy eqImpl(values::RValue const &lhs, values::RValue const &rhs, API_CTX);
  SlotProxy neqImpl(values::RValue const &lhs, values::RValue const &rhs, API_CTX);
  SlotProxy ltImpl(values::RValue const &lhs, values::RValue const &rhs, API_CTX);
  SlotProxy leImpl(values::RValue const &lhs, values::RValue const &rhs, API_CTX);
  SlotProxy gtImpl(values::RValue const &lhs, values::RValue const &rhs, API_CTX);
  SlotProxy geImpl(values::RValue const &lhs, values::RValue const &rhs, API_CTX);


  void addAssignImpl(values::LValue const &lhs, values::RValue const &rhs, API_CTX);
  void subAssignImpl(values::LValue const &lhs, values::RValue const &rhs, API_CTX);
  void mulAssignImpl(values::LValue const &lhs, values::RValue const &rhs, API_CTX);
  void divAssignImpl(values::LValue const &lhs, values::RValue const &rhs, API_CTX);
  void modAssignImpl(values::LValue const &lhs, values::RValue const &rhs, API_CTX);

  void branchIfImpl(values::RValue const &condition, std::string const &trueLabel,
			   std::string const &falseLabel, API_CTX);
  void assignImpl(values::LValue const &lhs, values::RValue const &rhs, API_CTX);
  void writeOutImpl(values::RValue const &rhs, API_CTX); 
  SlotProxy addressOfImpl(values::LValue const &obj, API_CTX);
  
  // Slot operations
  void assignSlot(Slot const &dest, Slot const &src);
  void assignSlot(Slot const &slot, values::Literal const &val);
  void addSlotToSlot(Slot const &lhs, Slot const &rhs);
  void addConstToSlot(Slot const &lhs, int delta);
  void subSlotFromSlot(Slot const &lhs, Slot const &rhs);
  void subConstFromSlot(Slot const &lhs, int delta);
  void mulSlotBySlot(Slot const &lhs, Slot const &rhs);
  void mulSlotByConst(Slot const &lhs, int factor);
  void divSlotByConst(Slot const &lhs, int denom);
  void divSlotBySlot(Slot const &lhs, Slot const &rhs);
  void modSlotByConst(Slot const &lhs, int denom);
  void modSlotBySlot(Slot const &lhs, Slot const &rhs);

  void branchIfSlot(Slot const &slot, std::string const &trueLabel, std::string const &falseLabel);
  void copySlotIntoElement(Slot const &srcSlot, Slot const &arrSlot, Slot const &indexSlot);
  void copyElementIntoSlot(Slot const &elementSlot, Slot const &arrSlot, Slot const &indexSlot);
  void dereferencePointerIntoSlot(Slot const &ptrSlot, Slot const &derefSlot);
  void writeSlotThroughDereferencedPointer(Slot const &ptrSlot, Slot const &srcSlot);
  Slot addressOfSlot(Slot const &slot);
  
  // Algorithms: all applied to the current DP (compiler_algorithms.cc)
  void moveTo(Cell cell);
  void moveTo(int offset, MacroCell::Field field = MacroCell::Value0);
  void moveToOrigin();
  void moveRel(int diff);
  void switchField(MacroCell::Field field);  
  void zeroCell();
  void zeroCellPlus();
  void loopOpen(std::string const &tag = defaultOpenTag());
  void loopClose(std::string const &tag = defaultCloseTag());

  void goToDynamicOffset(Cell offsetLow, Cell offsetHigh);
  void fetchFromDynamicOffset(Cell offsetLow, Cell offsetHigh, Payload const &payload, primitive::Direction seekDir);
  
  void moveField(Cell dest);
  void copyField(Cell dest, Temps<1>);
  
  void setToValue(int value);
  void setToValue16(int value, Cell high);

  void inc();
  void dec();
  void inc16(Cell high, Temps<2>);
  void dec16(Cell high, Temps<2>);
  

  // TODO: constructive versions should accept "other" before result and carry
  void addConst(int delta);
  void addConstAndCarry(int delta, Cell carry, Temps<3>);
  void add16Const(int delta, Cell high, Temps<4>);
  void addDestructive(Cell other);
  void addConstructive(Cell result, Cell other, Temps<2>);
  void add16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<4>);
  void add16Constructive(Cell high, Cell resultLow, Cell resultHigh, Cell otherLow, Cell otherHigh, Temps<6>);
  void addAndCarryDestructive(Cell carry, Cell other, Temps<3>);
  void addAndCarryConstructive(Cell result, Cell carry, Cell other, Temps<4>);

  void subConst(int delta);
  void subConstAndCarry(int delta, Cell carry, Temps<3>);
  void sub16Const(int delta, Cell high, Temps<4>);
  void subDestructive(Cell other);
  void subConstructive(Cell result, Cell other, Temps<2>);
  void sub16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<4>);
  void sub16Constructive(Cell high, Cell resultLow, Cell resultHigh, Cell otherLow, Cell otherHigh, Temps<6>);
  void subAndCarryDestructive(Cell carry, Cell other, Temps<3>);
  void subAndCarryConstructive(Cell result, Cell carry, Cell other, Temps<4>);

  void mulConst(int factor, Temps<3>);
  void mul16Const(int factor, Cell high, Temps<8>);
  void mulDestructive(Cell other, Temps<3>);
  void mulConstructive(Cell result, Cell factor, Temps<4>);
  void mul16Destructive(Cell high, Cell factorLow, Cell factorHigh, Temps<9>);
  void mul16Constructive(Cell high, Cell resultLow, Cell resultHigh, Cell factorLow, Cell factorHigh, Temps<11>);

  void divModConst(int denom, Cell modResult, Temps<5>);
  void divMod16Const(int denom, Cell high, Cell modResultLow, Cell modResultHigh, Temps<8>);
  void divModDestructive(Cell denom, Cell modResult, Temps<5>);
  void divModConstructive(Cell result, Cell denom, Cell modResult, Temps<6>);
  void divMod16Destructive(Cell high, Cell denomLow, Cell denomHigh, Cell modResultLow, Cell modResultHigh, Temps<8>);
  void divMod16Constructive(Cell high, Cell resultLow, Cell resultHigh, Cell denomLow, Cell denomHigh, Cell modResultLow, Cell modResultHigh, Temps<12>);

  void notDestructive(Temps<1>);
  void notConstructive(Cell result, Temps<1>);
  void not16Destructive(Cell high, Temps<1>);
  void not16Constructive(Cell high, Cell result, Temps<2>);

  void orDestructive(Cell other, Temps<1>);
  void orConstructive(Cell result, Cell other, Temps<2>);

  void andDestructive(Cell other, Temps<1>);
  void andConstructive(Cell result, Cell other, Temps<2>);

  void compareToConstDestructive(int value, Temps<1>);
  void compareToConstConstructive(int value, Cell result, Temps<1>);    

  void compare16ToConstDestructive(int value, Cell high, Temps<1>);
  void compare16ToConstConstructive(int value, Cell high, Cell result, Temps<2>);    
  
  void lessDestructive(Cell other, Temps<2>);
  void lessConstructive(Cell result, Cell other, Temps<3>);

  void lessOrEqualDestructive(Cell other, Temps<2>);
  void lessOrEqualConstructive(Cell result, Cell other, Temps<3>);

  void greaterDestructive(Cell other, Temps<2>);
  void greaterConstructive(Cell result, Cell other, Temps<3>);

  void greaterOrEqualDestructive(Cell other, Temps<2>);
  void greaterOrEqualConstructive(Cell result, Cell other, Temps<3>);

  void equalDestructive(Cell other, Temps<2>);
  void equalConstructive(Cell result, Cell other, Temps<3>);
  
  // Frame Navigation (compiler_framenav.cc)
  
  void pushFrame();
  void popFrame();
  void seek(MacroCell::Field markerField, primitive::Direction dir, Payload const &payload, bool checkCurrent);
  void setSeekMarker();
  void resetSeekMarker();
  void moveToPreviousFrame(Payload const &payload = {});  
  void initializeArguments(std::string const &functionName, std::vector<values::RValue> const &args, API_CTX);  
  void fetchReturnData();
  void fetchReturnData(Slot const &returnSlot);

  // Temporaries and memory management (compiler_memory.cc)
  void freeSlot(Slot &slot);
  void freeTemps();
  void freeScope(Function::Scope const *scope);
  Slot allocSlot(std::string const &name, types::TypeHandle type, Slot::Kind kind);
  Slot getTemp(types::TypeHandle type);
  Slot getTemp(values::Literal const &val);
  void swapLocalWithTemp(Slot const &local, Slot const &tmp);
  
  // Global Data Synchronization (compiler_globals.cc)
  void fetchGlobal(Slot const &globalSlot, Slot const &localSlot);
  void putGlobal(Slot const &globalSlot, Slot const &localSlot);

  template <auto FetchOrPut>
  void syncGlobal(Slot const &localSlot, bool onlyAliasedGlobals = false) {
    assert(localSlot.kind == Slot::GlobalReference);
    
    std::string globalName = localSlot.name.substr(std::string("__g_").size());
    assert(_program.isGlobal(globalName));
    if (onlyAliasedGlobals && not _aliasedGlobals.contains(globalName)) return;
    
    Slot const &globalSlot = _program.globalSlot(globalName);
    assert(globalSlot.size() == localSlot.size());

    (this->*FetchOrPut)(globalSlot, localSlot);
  }  

  template <auto FetchOrPut>
  void syncGlobals(bool onlyAliasedGlobals = false) {
    auto const &locals = _currentFunction->frame.locals;
    for (auto const &localSlot: locals) {
      if (localSlot.kind != Slot::GlobalReference) continue;
      syncGlobal<FetchOrPut>(localSlot, onlyAliasedGlobals);
    }  
  }

  void syncGlobalToLocal(bool onlyAliasedGlobals = false) {
    syncGlobals<&Compiler::fetchGlobal>(onlyAliasedGlobals);
  }

  void syncLocalToGlobal(bool onlyAliasedGlobals = false) {
    syncGlobals<&Compiler::putGlobal>(onlyAliasedGlobals);
  }
  
  // Codeblock construction (compiler_codeblocks.cc)
  void blockOpen();
  void blockClose();
  void constructMetaBlocks();

  // Code generation (compiler_codegen.cc)
  void setTargetSequence(primitive::Sequence *seq);
  primitive::Context constructContext() const;    
  primitive::Sequence compilePrimitives() const;
  
  // Pointer management (compiler_misc.cc)
  void resetOrigin();
  void pushPtr();
  void popPtr();

  // Post processing/optimization (compiler_misc.cc)
  void deferFunctionCallTypeCheck(std::string const &caller, std::string const &callee,
				  std::vector<values::RValue> const &args, API_CTX);

  void functionCallTypeChecks();
  
  static std::string simplifyProgram(std::string const &bf);
  
  // General helpers (inline definitions)
  static inline std::string defaultOpenTag() {
    static int count = 0;
    return std::string("open_loop_") + std::to_string(count++);
  } 
  static inline std::string defaultCloseTag() {
    static int count = 0;
    return std::string("close_loop_") + std::to_string(count++);
  }
  
  template <typename Primitive, typename ... Args>
  inline void emit(Args&& ... args) {
    assert(_currentSeq != nullptr);
    _currentSeq->emplace<Primitive>(std::forward<Args>(args)...);
  }

  inline int getFieldIndex(int offset, int field) {
    return offset * MacroCell::FieldCount + field;
  }

  inline int getFieldIndex(Cell cell) {
    return getFieldIndex(cell.offset, cell.field);
  }

  template <typename... Args> requires ((std::convertible_to<Args, Cell>) && ...)
  auto getFieldIndices(Args... args) {
    return std::make_tuple(getFieldIndex(static_cast<Cell>(args))...);
  }

  // // Helper for creating argument vector
  // template <typename ... Args> 
  // std::vector<values::RValue> constructArgumentList(Args&& ... args) {
  //   std::vector<values::RValue> result; result.reserve(sizeof...(args));
  //   (result.emplace_back(rValue(std::forward<Args>(args))), ...);
  //   return result;
  // }
  
  
  // // Error Handling
  // struct Error: std::exception {

  //   std::string msg;
  //   Error(std::string const &msgHead): msg(msgHead) {}
    
  //   template <typename T>
  //   Error &operator<<(T const &val) {
  //     std::ostringstream oss;
  //     oss << val;
  //     msg += oss.str();
  //     return *this;
  //   }

  //   virtual char const *what() const noexcept override {
  //     return msg.c_str();
  //   }
  // };

  // template <typename ... Args> requires (sizeof...(Args) > 0)
  // void error(Args&& ... args) const {
  //   Error err("Backend error: "); (err << ... << args);
  //   throw err;
  // }

  // template <typename ... Args> requires (sizeof...(Args) > 0)
  // void warning(Args&& ... args) const {
  //   Error err("Backend warning: "); (err << ... << args);
  //   throw err;
  // }
  
  // template <typename ... Args> requires (sizeof...(Args) > 0)
  // void error_if(bool const condition, Args&& ... args) const {
  //   if (condition) error(std::forward<Args>(args)...);
  // }

  // template <typename ... Args> requires (sizeof...(Args) > 0)
  // void warning_if(bool const condition, Args&& ... args) const {
  //   if (condition) warning(std::forward<Args>(args)...);
  // }

  
  
};
