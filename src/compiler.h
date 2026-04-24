#pragma once
#include <stack>
#include <sstream>
#include <optional>
#include <unordered_set>
#include "proxy.h"
#include "program.h"
#include "function.h"
#include "data.h"
#include "expressionresult.h"
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

  // TODO: there must be a cleaner way to pass args
  using ArgList = std::vector<ExpressionResult>;
  
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
  void beginFunction(std::string const &name, API_FUNC);
  void beginFunction(std::string const &name, types::TypeHandle funcType, API_FUNC);
  void beginFunction(std::string const &name, types::TypeHandle funcType, std::vector<std::string> const &params, API_FUNC);
  void setNextBlock(int index, API_FUNC);
  void setNextBlock(std::string const &b, API_FUNC);
  void setNextBlock(std::string const &f, std::string const &b, API_FUNC);
  void abortProgram(API_FUNC);
  void referGlobals(std::vector<std::string> const &names, API_FUNC);
  Slot local(std::string const& name, bool globalReference = false) const;  
  Slot declareLocal(std::string const &name, types::TypeHandle type, API_FUNC);
  Slot declareGlobal(std::string const &name, types::TypeHandle type, API_FUNC);
  Slot declareGlobalReference(Slot const &globalSlot);
  void callFunction(std::string const& functionName, std::string const& nextBlockName, API_FUNC);  
  void callFunction(std::string const& functionName, std::string const& nextBlockName, ArgList const &args, API_FUNC);
  void returnFromFunction(API_FUNC);


  // TODO: there must be a cleaner way to define a struct
  template <typename ... Args> StructFields constructFields(Args&& ... args);
  types::TypeHandle defineStruct(std::string const& name, StructFields const &fields, API_FUNC);

  template <typename... Args> ArgList constructFunctionArguments_(API_FUNC_SOURCE, Args&&... args);  
  
  template <typename Ret> void callFunction(std::string const& functionName, std::string const& nextBlockName,
					    ArgList const &args, Ret const &returnSlot, API_FUNC);

  template <typename Ret> void returnFromFunction(Ret const &ret, API_FUNC);

  template <typename Obj> ExpressionResult structField(Obj const &obj, std::string const &field, API_FUNC);
  template <typename Obj> ExpressionResult structField(Obj const &obj, int fieldIndex, API_FUNC);
  template <typename Ptr> ExpressionResult dereferencePointer(Ptr const &ptr, API_FUNC);
  template <typename Arr> ExpressionResult arrayElement(Arr const &arr, int index, API_FUNC);  
  template <typename Arr,
	    typename Idx> ExpressionResult arrayElement(Arr const &arr, Idx const &index, API_FUNC);


  template <typename L, typename R> ExpressionResult addAssign(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult subAssign(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult mulAssign(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult divAssign(L const &lhs, R const &rhs, API_FUNC);  
  template <typename L, typename R> ExpressionResult modAssign(L const &lhs, R const &rhs, API_FUNC);  

  template <typename L, typename R> ExpressionResult add(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult sub(L const &lhs, R const &rhs, API_FUNC);  
  template <typename L, typename R> ExpressionResult mul(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult div(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult mod(L const &lhs, R const &rhs, API_FUNC);  

  template <typename L, typename R> ExpressionResult landAssign(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult lnandAssign(L const &lhs, R const &rhs, API_FUNC);  
  template <typename L, typename R> ExpressionResult lorAssign(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult lnorAssign(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult lxorAssign(L const &lhs, R const &rhs, API_FUNC);  
  template <typename L, typename R> ExpressionResult lxnorAssign(L const &lhs, R const &rhs, API_FUNC);  

  template <typename L, typename R> ExpressionResult land(L const &lhs, R const &rhs, API_FUNC);  
  template <typename L, typename R> ExpressionResult lnand(L const &lhs, R const &rhs, API_FUNC);  
  template <typename L, typename R> ExpressionResult lor(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult lnor(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult lxor(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult lxnor(L const &lhs, R const &rhs, API_FUNC);
  
  template <typename L, typename R> ExpressionResult eqAssign(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult neqAssign(L const &lhs, R const &rhs, API_FUNC);  
  template <typename L, typename R> ExpressionResult ltAssign(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult leAssign(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult gtAssign(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult geAssign(L const &lhs, R const &rhs, API_FUNC);  

  template <typename L, typename R> ExpressionResult eq(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult neq(L const &lhs, R const &rhs, API_FUNC);  
  template <typename L, typename R> ExpressionResult lt(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult le(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult gt(L const &lhs, R const &rhs, API_FUNC);
  template <typename L, typename R> ExpressionResult ge(L const &lhs, R const &rhs, API_FUNC);  
  
  template <typename L, typename R> ExpressionResult assign(L const &lhs, R const &rhs, API_FUNC);
  template <typename Val> void writeOut(Val const &val, API_FUNC);
  
  template <typename Obj> ExpressionResult addressOf(Obj const &obj, API_FUNC);
  template <typename Con> void branchIf(Con const &condition, std::string const &trueLabel,
					std::string const &falseLabel, API_FUNC);
  
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
  ExpressionResult rValue(ExpressionResult const &val, API_CTX) const;
  ExpressionResult rValue(std::string const &var, API_CTX) const;
  ExpressionResult rValue(SlotProxy const &slot, API_CTX) const;
  ExpressionResult rValue(values::Literal const &val, API_CTX) const;

  ExpressionResult lValue(ExpressionResult const &val, API_CTX) const;
  ExpressionResult lValue(std::string const &var, API_CTX) const;  
  ExpressionResult lValue(SlotProxy const &slot, API_CTX) const;

  // Implementation functions for public interface
  void setNextBlockImpl(int index);
  void setNextBlockImpl(std::string const &f, std::string const &b);

  void callFunctionImpl(std::string const& functionName, std::string const& nextBlockName,
			std::optional<ExpressionResult> const &returnSlot, ArgList const &args, API_CTX);
  void returnFromFunctionImpl(std::optional<ExpressionResult> const &ret, API_CTX);
  ExpressionResult structFieldImpl(ExpressionResult const &obj, std::string const &field, API_CTX);
  ExpressionResult structFieldImpl(ExpressionResult const &obj, int fieldIndex, API_CTX);
  ExpressionResult arrayElementImpl(ExpressionResult const &arr, int index, API_CTX);
  ExpressionResult arrayElementImpl(ExpressionResult const &arr, ExpressionResult const &index, API_CTX);
  ExpressionResult dereferencePointerImpl(ExpressionResult const &ptr, API_CTX);

  ExpressionResult addressOfImpl(ExpressionResult const &obj, API_CTX);
  ExpressionResult assignImpl(ExpressionResult const &lhs, ExpressionResult const &rhs, API_CTX);
  
  void branchIfImpl(ExpressionResult const &condition, std::string const &trueLabel, std::string const &falseLabel, API_CTX);
  void writeOutImpl(ExpressionResult const &rhs, API_CTX); 
  
  // Binary operators
  template <typename Fold>
  struct BinOpSpec {
    using MaterializeFunc = Slot(*)(ExpressionResult const &);
    using ApplyWithSlot   = void(Compiler::*)(Slot const &, Slot const &);
    using ApplyWithConst  = void(Compiler::*)(Slot const &, int);

    BinOp op;
    Fold fold;
    ApplyWithSlot applyWithSlot;
    ApplyWithConst applyWithConst;

  };

  using Lop = BinOpSpec<bool(*)(bool, bool)>;
  using Mop = BinOpSpec<int(*)(int, int)>;
  using Cop = BinOpSpec<bool(*)(int, int)>;

  template <typename SpecType>
  ExpressionResult opAssignImpl(ExpressionResult const &lhs, ExpressionResult const &rhs, SpecType const &lop, API_CTX);

  template <typename SpecType>  
  ExpressionResult opImpl(ExpressionResult const &lhs, ExpressionResult const &rhs, SpecType const &lop, API_CTX);
  
#define OP_DECL(Type, OP)						\
  ExpressionResult OP##Impl(ExpressionResult const &lhs, ExpressionResult const &rhs, API_CTX); \
  ExpressionResult OP##AssignImpl(ExpressionResult const &lhs, ExpressionResult const &rhs, API_CTX); \
  static Type const OP##Spec;
  
  // Mathmatical operators
  OP_DECL(Mop, add);
  OP_DECL(Mop, sub);
  OP_DECL(Mop, mul);
  OP_DECL(Mop, div);
  OP_DECL(Mop, mod);

  // Logical operators
  OP_DECL(Lop, land);
  OP_DECL(Lop, lnand);
  OP_DECL(Lop, lor);
  OP_DECL(Lop, lnor);
  OP_DECL(Lop, lxor);
  OP_DECL(Lop, lxnor);

  // Comparison operators
  OP_DECL(Cop, eq);
  OP_DECL(Cop, neq);
  OP_DECL(Cop, lt);
  OP_DECL(Cop, le);
  OP_DECL(Cop, gt);
  OP_DECL(Cop, ge);
#undef OP_DECL
  
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

  void andSlotWithConst(Slot const &lhs, int val);
  void andSlotWithSlot(Slot const &lhs, Slot const &rhs);
  void nandSlotWithConst(Slot const &lhs, int val);
  void nandSlotWithSlot(Slot const &lhs, Slot const &rhs);
  void orSlotWithConst(Slot const &lhs, int val);
  void orSlotWithSlot(Slot const &lhs, Slot const &rhs);
  void norSlotWithConst(Slot const &lhs, int val);
  void norSlotWithSlot(Slot const &lhs, Slot const &rhs);
  void xorSlotWithConst(Slot const &lhs, int val);
  void xorSlotWithSlot(Slot const &lhs, Slot const &rhs);
  void xnorSlotWithConst(Slot const &lhs, int val);
  void xnorSlotWithSlot(Slot const &lhs, Slot const &rhs);

  void slotEqualConst(Slot const &lhs, int val);
  void slotEqualSlot(Slot const &lhs, Slot const &rhs);
  void slotNotEqualConst(Slot const &lhs, int val);
  void slotNotEqualSlot(Slot const &lhs, Slot const &rhs);
  void slotLessConst(Slot const &lhs, int val);
  void slotLessSlot(Slot const &lhs, Slot const &rhs);
  void slotLessEqualConst(Slot const &lhs, int val);
  void slotLessEqualSlot(Slot const &lhs, Slot const &rhs);
  void slotGreaterConst(Slot const &lhs, int val);
  void slotGreaterSlot(Slot const &lhs, Slot const &rhs);
  void slotGreaterEqualConst(Slot const &lhs, int val);
  void slotGreaterEqualSlot(Slot const &lhs, Slot const &rhs);
  
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

  void boolDestructive(Temps<1>);
  void boolConstructive(Cell result, Temps<1>);
  void bool16Destructive(Cell high, Temps<1>);
  void bool16Constructive(Cell high, Cell result, Temps<2>);
  
  void notDestructive(Temps<1>);
  void notConstructive(Cell result, Temps<1>);
  void not16Destructive(Cell high, Temps<1>);
  void not16Constructive(Cell high, Cell result, Temps<2>);

  void orDestructive(Cell other, Temps<1>);
  void orConstructive(Cell result, Cell other, Temps<2>);
  void or16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<1>);
  void or16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<4>);

  void norDestructive(Cell other, Temps<1>);
  void norConstructive(Cell result, Cell other, Temps<2>);
  void nor16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<1>);
  void nor16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<4>);
  
  void andDestructive(Cell other, Temps<1>);
  void andConstructive(Cell result, Cell other, Temps<2>);
  void and16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<1>);
  void and16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<4>);

  void nandDestructive(Cell other, Temps<1>);
  void nandConstructive(Cell result, Cell other, Temps<2>);
  void nand16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<1>);
  void nand16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<4>);

  void xorDestructive(Cell other, Temps<1>);
  void xorConstructive(Cell result, Cell other, Temps<2>);
  void xor16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<1>);
  void xor16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<4>);

  void xnorDestructive(Cell other, Temps<1>);
  void xnorConstructive(Cell result, Cell other, Temps<2>);
  void xnor16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<1>);
  void xnor16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<4>);
  
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
  void initializeArguments(std::string const &functionName, std::vector<ExpressionResult> const &args, API_CTX);  
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
				  std::vector<ExpressionResult> const &args, API_CTX);

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
};

#include "compiler_public.tpp"
