#pragma once
#include <string>
#include <tuple>
#include <vector>
#include <utility>
#include <stack>
#include <optional>
#include <concepts>
#include <unordered_set>

#include "acus/core/proxy.h"
#include "acus/core/program.h"
#include "acus/core/data.h"
#include "acus/core/expression.h"
#include "acus/types/operators.h"
#include "acus/types/struct_field.h"
#include "acus/types/types.h"
#include "acus/types/values_fwd.h"

#define API_HEADER
#include "acus/api/api.h"

namespace acus {

// ============================================================
// Builder
// ============================================================

class Builder {
public:
  inline Builder() { TypeSystem::init(); }

  // TODO: API_FUNC 
  std::string dumpPrimitives() const;
  std::string dumpBrainfuck() const;

  // Builders
  void setEntryPoint(std::string functionName, API_FUNC);

  void begin(API_FUNC);
  void end(API_FUNC);

  void beginScope(API_FUNC);
  void endScope(API_FUNC);

  void beginBlock(std::string name, API_FUNC);
  void endBlock(API_FUNC);

  void beginFunction(std::string const &name, API_FUNC);
  void beginFunction(std::string const &name, types::TypeHandle funcType, API_FUNC);
  void beginFunction(std::string const &name, types::TypeHandle funcType, std::vector<std::string> const &params, API_FUNC);
  void endFunction(API_FUNC);

  void setNextBlock(std::string const &b, API_FUNC);
  void setNextBlock(std::string const &f, std::string const &b, API_FUNC);

  void referGlobals(std::vector<std::string> const &names, API_FUNC);
  Slot declareLocal(std::string const &name, types::TypeHandle type, API_FUNC);
  Slot declareGlobal(std::string const &name, types::TypeHandle type, API_FUNC);

  void returnFromFunction(auto const &ret, API_FUNC);
  void returnFromFunction(API_FUNC);
  void abortProgram(API_FUNC);

  struct FunctionCall;
  struct StructDefinition;
  
#define FUNCTION_CALL_OBJECT [[nodiscard("call the returned FunctionCall object with arguments, e.g. callFunction(...)(args...)")]] FunctionCall
#define STRUCT_DEFINE_OBJECT [[nodiscard("call the returned StructDefinition object with arguments, e.g. defineStruct(...)(args...)")]] StructDefinition

  FUNCTION_CALL_OBJECT callFunction(std::string const& functionName, std::string const& nextBlockName, auto const &returnSlot, API_FUNC);
  FUNCTION_CALL_OBJECT callFunction(std::string const& functionName, std::string const& nextBlockName, API_FUNC);  
  FUNCTION_CALL_OBJECT callFunctionPointer(auto const &functionPtr, std::string const& nextBlockName, API_FUNC);  
  FUNCTION_CALL_OBJECT callFunctionPointer(auto const &functionPtr, std::string const& nextBlockName, auto const &returnSlot, API_FUNC);  
  STRUCT_DEFINE_OBJECT defineStruct(std::string const& name, API_FUNC);

#undef FUNCTION_CALL_OBJECT
#undef STRUCT_DEFINE_OBJECT

  Expression expr(auto const &obj, API_FUNC);
  Expression assign(auto const &lhs, auto const &rhs, API_FUNC);

  Expression structField(auto const &obj, std::string const &field, API_FUNC);
  Expression structField(auto const &obj, int fieldIndex, API_FUNC);
  Expression dereferencePointer(auto const &ptr, API_FUNC);
  Expression arrayElement(auto const &arr, int index, API_FUNC);  
  Expression arrayElement(auto const &arr, auto const &index, API_FUNC);

    // TODO: implement lnot
  Expression lnotAssign(auto const &rhs, API_FUNC);
  Expression lnot(auto const &rhs, API_FUNC);

  Expression binOp(BinOp op, auto const &lhs, auto const &rhs, API_FUNC);
  Expression binOpAssign(BinOp op, auto const &lhs, auto const &rhs, API_FUNC);

  Expression addAssign(auto const &lhs, auto const &rhs, API_FUNC);
  Expression subAssign(auto const &lhs, auto const &rhs, API_FUNC);
  Expression mulAssign(auto const &lhs, auto const &rhs, API_FUNC);
  Expression divAssign(auto const &lhs, auto const &rhs, API_FUNC);  
  Expression modAssign(auto const &lhs, auto const &rhs, API_FUNC);  

  Expression add(auto const &lhs, auto const &rhs, API_FUNC);
  Expression sub(auto const &lhs, auto const &rhs, API_FUNC);  
  Expression mul(auto const &lhs, auto const &rhs, API_FUNC);
  Expression div(auto const &lhs, auto const &rhs, API_FUNC);
  Expression mod(auto const &lhs, auto const &rhs, API_FUNC);  
  
  Expression landAssign(auto const &lhs, auto const &rhs, API_FUNC);
  Expression lnandAssign(auto const &lhs, auto const &rhs, API_FUNC);  
  Expression lorAssign(auto const &lhs, auto const &rhs, API_FUNC);
  Expression lnorAssign(auto const &lhs, auto const &rhs, API_FUNC);
  Expression lxorAssign(auto const &lhs, auto const &rhs, API_FUNC);  
  Expression lxnorAssign(auto const &lhs, auto const &rhs, API_FUNC);  
  
  Expression land(auto const &lhs, auto const &rhs, API_FUNC);  
  Expression lnand(auto const &lhs, auto const &rhs, API_FUNC);  
  Expression lor(auto const &lhs, auto const &rhs, API_FUNC);
  Expression lnor(auto const &lhs, auto const &rhs, API_FUNC);
  Expression lxor(auto const &lhs, auto const &rhs, API_FUNC);
  Expression lxnor(auto const &lhs, auto const &rhs, API_FUNC);

  Expression eqAssign(auto const &lhs, auto const &rhs, API_FUNC);
  Expression neqAssign(auto const &lhs, auto const &rhs, API_FUNC);  
  Expression ltAssign(auto const &lhs, auto const &rhs, API_FUNC);
  Expression leAssign(auto const &lhs, auto const &rhs, API_FUNC);
  Expression gtAssign(auto const &lhs, auto const &rhs, API_FUNC);
  Expression geAssign(auto const &lhs, auto const &rhs, API_FUNC);  

  Expression eq(auto const &lhs, auto const &rhs, API_FUNC);
  Expression neq(auto const &lhs, auto const &rhs, API_FUNC);  
  Expression lt(auto const &lhs, auto const &rhs, API_FUNC);
  Expression le(auto const &lhs, auto const &rhs, API_FUNC);
  Expression gt(auto const &lhs, auto const &rhs, API_FUNC);
  Expression ge(auto const &lhs, auto const &rhs, API_FUNC);  

  Expression addressOf(auto const &obj, API_FUNC);

  void writeOut(auto const &val, API_FUNC);
  void branchIf(auto const &condition, std::string const &trueLabel, std::string const &falseLabel, API_FUNC);
  
private:
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

  std::stack<Cell> _ptrStack;
  DataPointer _dp;

  struct MetaBlock {
    std::string name;
    std::string caller;
    std::variant<std::string, types::FunctionType const *> callee;
    types::TypeHandle returnType;
    std::optional<SlotProxy> returnSlot;
    std::string nextBlockName;
  };
  std::vector<MetaBlock> _metaBlocks;


  struct FunctionCallInfo {
    api::Context API_CTX_NAME;
    std::string callee;
    std::vector<Expression> args;
  };
  std::vector<FunctionCallInfo> _deferredFunctionCallTypeChecks;

  struct BlockNameCheck {
    api::Context API_CTX_NAME;
    std::string functionName, blockName;
  };
  std::vector<BlockNameCheck> _deferredBlockNameChecks;

  std::unordered_set<std::string> _aliasedGlobals;
  
  // Diagnostics (builder_diag.cc)
  std::string currentFunction() const;
  std::string currentBlock() const;
  bool programStarted() const;
  bool declaredAsGlobal(std::string const &name) const;
  bool globalDeclarationsAllowed() const;
  bool inScope(std::string const &name) const;
  bool inCurrentScope(std::string const &name) const;
  int currentScopeDepth() const;
  
  // Normalize to RValue or LValue (builder_rlvalue.cc)
  Expression rValue(Expression const &val, API_CTX) const;
  Expression rValue(std::string const &var, API_CTX) const;
  Expression rValue(SlotProxy const &slot, API_CTX) const;
  Expression rValue(values::Literal const &val, API_CTX) const;

  Expression lValue(Expression const &val, API_CTX) const;
  Expression lValue(std::string const &var, API_CTX) const;  
  Expression lValue(SlotProxy const &slot, API_CTX) const;

  // Implementation functions for public interface
  void setNextBlockImpl(int index);
  void setNextBlockImpl(std::string const &f, std::string const &b);
  void setNextBlockImpl(Expression const &obj);

  types::TypeHandle defineStructImpl(std::string const& name, std::vector<NameTypePair> const &fields, API_CTX);

  void callFunctionImpl(std::string const &functionName, std::string const& nextBlockName,
			std::optional<Expression> const &returnSlot, std::vector<Expression> const &args, API_CTX);
  void callFunctionImpl(Expression const &functionPointer, std::string const& nextBlockName,
			std::optional<Expression> const &returnSlot, std::vector<Expression> const &args, API_CTX);
  void returnFromFunctionImpl(std::optional<Expression> const &ret, API_CTX);
  Expression structFieldImpl(Expression const &obj, std::string const &field, API_CTX);
  Expression structFieldImpl(Expression const &obj, int fieldIndex, API_CTX);
  Expression arrayElementImpl(Expression const &arr, int index, API_CTX);
  Expression arrayElementImpl(Expression const &arr, Expression const &index, API_CTX);
  Expression dereferencePointerImpl(Expression const &ptr, API_CTX);

  Expression addressOfImpl(Expression const &obj, API_CTX);
  Expression assignImpl(Expression const &lhs, Expression const &rhs, API_CTX);
  
  void branchIfImpl(Expression const &condition, std::string const &trueLabel, std::string const &falseLabel, API_CTX);
  void writeOutImpl(Expression const &rhs, API_CTX); 
  
  // Binary operators implementation
  template <typename Fold>
  struct BinOpSpec {
    using ApplyWithSlot   = void(Builder::*)(Slot const &, Slot const &);
    using ApplyWithConst  = void(Builder::*)(Slot const &, int);

    BinOp op;
    Fold *fold;
    ApplyWithSlot applyWithSlot;
    ApplyWithConst applyWithConst;
  };

  using Mop = BinOpSpec<int(int, int)>;
  using Lop = BinOpSpec<bool(bool, bool)>;
  using Cop = BinOpSpec<bool(int, int)>;

  static const Mop addSpec, subSpec, mulSpec, divSpec, modSpec;
  static const Lop landSpec, lnandSpec, lorSpec, lnorSpec, lxorSpec, lxnorSpec;
  static const Cop eqSpec, neqSpec, ltSpec, leSpec, gtSpec, geSpec;
  
  template <typename SpecType>
  Expression opAssignImpl(Expression const &lhs, Expression const &rhs, SpecType const &spec, API_CTX);

  template <typename SpecType>  
  Expression opImpl(Expression const &lhs, Expression const &rhs, SpecType const &spec, API_CTX);
  
  // Slot operations
  Slot local(std::string const& name, bool globalReference = false) const;
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
  
  // Algorithms: all applied to the current DP (builder_algorithms.cc)
  void moveTo(Cell cell);
  void moveTo(int offset, MacroCell::Field field = MacroCell::Value0);
  void moveToOrigin();
  void moveRel(int diff);
  void switchField(MacroCell::Field field);  
  void zeroCell();
  void zeroCellPlus();
  void loopOpen(std::string const &tag = defaultOpenTag());
  void loopClose(std::string const &tag = defaultCloseTag());

  void goToDynamicOffset(Cell offsetLow, Cell offsetHigh);// TODO: rename moveTo..
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

  void xorDestructive(Cell other, Temps<2>);
  void xorConstructive(Cell result, Cell other, Temps<3>);
  void xor16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<2>);
  void xor16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<5>);

  void xnorDestructive(Cell other, Temps<2>);
  void xnorConstructive(Cell result, Cell other, Temps<3>);
  void xnor16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<2>);
  void xnor16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<5>);

  void compareToConstDestructive(int value, Temps<1>);
  void compareToConstConstructive(int value, Cell result, Temps<1>);    
  void compare16ToConstDestructive(int value, Cell high, Temps<1>);
  void compare16ToConstConstructive(int value, Cell high, Cell result, Temps<2>);    

  void eqDestructive(Cell other, Temps<1>);
  void eqConstructive(Cell result, Cell other, Temps<1>);
  void eq16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<1>);
  void eq16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<4>);
  
  void lessDestructive(Cell other, Temps<2>);
  void lessConstructive(Cell result, Cell other, Temps<3>);
  void less16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<4>);
  void less16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<8>);

  void lessOrEqualDestructive(Cell other, Temps<2>);
  void lessOrEqualConstructive(Cell result, Cell other, Temps<3>);
  void lessOrEqual16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<4>);
  void lessOrEqual16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<8>);

  void greaterDestructive(Cell other, Temps<2>);
  void greaterConstructive(Cell result, Cell other, Temps<3>);
  void greater16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<4>);
  void greater16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<8>);

  void greaterOrEqualDestructive(Cell other, Temps<2>);
  void greaterOrEqualConstructive(Cell result, Cell other, Temps<3>);
  void greaterOrEqual16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<4>);
  void greaterOrEqual16Constructive(Cell high, Cell result, Cell otherLow, Cell otherHigh, Temps<8>);

  
  // Frame Navigation (builder_framenav.cc)
  void resetOrigin();
  void pushPtr();
  void popPtr();  
  void pushFrame();
  void popFrame();
  void seek(MacroCell::Field markerField, primitive::Direction dir, Payload const &payload, bool checkCurrent);
  void setSeekMarker();
  void resetSeekMarker();
  void moveToPreviousFrame(Payload const &payload = {});  
  void initializeArguments(primitive::DInt const currentFrameSize, primitive::DInt const paramOffset, std::vector<Expression> const &args, API_CTX);
  void prepareNextFrame(std::string const &functionName, std::vector<Expression> const &args, API_CTX);
  void prepareNextFrame(Expression const &fptr, std::vector<Expression> const &args, API_CTX);
  
  //  void initializeArguments(Expression const &fptr, std::vector<Expression> const &args, API_CTX);
  void fetchReturnData();
  void fetchReturnData(Slot const &returnSlot);
  void moveToPointee(Slot const &ptrSlot);


  // Temporaries and memory management (builder_memory.cc)
  void freeSlot(Slot &slot);
  void freeTemps();
  void freeScope(Function::Scope const *scope);
  Slot allocSlot(std::string const &name, types::TypeHandle type, Slot::Kind kind);
  Slot getTemp(types::TypeHandle type);
  Slot getTemp(values::Literal const &val);
  void swapLocalWithTemp(Slot const &local, Slot const &tmp);
  Slot declareGlobalReference(Slot const &globalSlot);
  
  // Global Data Synchronization (builder_globals.cc)
  void fetchGlobal(Slot const &globalSlot, Slot const &localSlot);
  void putGlobal(Slot const &globalSlot, Slot const &localSlot);

  template <auto FetchOrPut>
  void syncGlobal(Slot const &localSlot, bool onlyAliasedGlobals = false);

  template <auto FetchOrPut>
  void syncGlobals(bool onlyAliasedGlobals = false);

  void syncGlobalToLocal(bool onlyAliasedGlobals = false);
  void syncLocalToGlobal(bool onlyAliasedGlobals = false);
  
  // Codeblock construction (builder_codeblocks.cc)
  void blockOpen();
  void blockClose();
  void constructMetaBlocks();

  // Code generation (builder_codegen.cc)
  void setTargetSequence(primitive::Sequence *seq);
  primitive::Context constructContext() const;    
  primitive::Sequence compilePrimitives() const;
  static std::string simplifyProgram(std::string const &bf);

  // Function call and block name checks
  void functionCallTypeCheck(types::FunctionType const *functionType, std::vector<Expression> const &args, API_CTX);
  void deferFunctionCallTypeCheck(std::string const &callee, std::vector<Expression> const &args, API_CTX);
  void deferredFunctionCallTypeChecks();

  void blockNameCheck(std::string const &functionName, std::string const &blockName, API_CTX);
  void deferBlockNameCheck(std::string const &f, std::string const &b, API_CTX);
  void deferredBlockNameChecks();

  // General helpers (inline definitions) TODO: move definition to builder_private.tpp
  static inline std::string defaultOpenTag() {
    static int count = 0;
    return std::string("open_loop_") + std::to_string(count++);
  } 
  static inline std::string defaultCloseTag() {
    static int count = 0;
    return std::string("close_loop_") + std::to_string(count++);
  }
  
  template <typename Primitive, typename ... Args>
  void emit(Args&& ... args);

  inline int getFieldIndex(int offset, int field) {
    return offset * MacroCell::FieldCount + field;
  }

  inline int getFieldIndex(Cell cell) {
    return getFieldIndex(cell.offset, cell.field);
  }

  template <typename... Args> requires ((std::convertible_to<Args, Cell>) && ...)
  auto getFieldIndices(Args... args);
};


// Builder objects for calls and struct definitions

class Builder::FunctionCall {
public:
  void operator()(auto const&... args) && {
    std::vector<Expression> argList;
    (argList.emplace_back(_builder.rValue(std::forward<decltype(args)>(args), API_FWD)), ...);
    std::move(*this)(argList);
  }

  void operator()(std::vector<Expression> const &argList) && {
    if (std::holds_alternative<std::string>(_function)) {
      _builder.callFunctionImpl(std::get<std::string>(_function), _nextBlockName, _return, argList, API_FWD);
    } else {
      _builder.callFunctionImpl(std::get<Expression>(_function), _nextBlockName, _return, argList, API_FWD);
    }

    _called = true;
  }
  
  ~FunctionCall() noexcept(false) {
    API_REQUIRE(_called, "operator() must be called on result of callFunction(); e.g. callFunction(\"foo\", \"after_foo\")(\"x\", \"y\");");
  }

private:
  friend class Builder;  
  Builder& _builder;
  std::variant<std::string, Expression> _function;
  std::string _nextBlockName;
  std::optional<Expression> _return;
  api::Context API_CTX_NAME;
  bool _called = false;

  FunctionCall(Builder &b, auto const &function, std::string const &nextBlockName,
	       std::optional<Expression> ret, api::Context const &ctx):
    _builder(b),
    _function(function),
    _nextBlockName(nextBlockName),
    _return(std::move(ret)),
    API_CTX_NAME(ctx)
  {}
    
  FunctionCall(FunctionCall const&) = delete;
  FunctionCall(FunctionCall &&) = delete;
  FunctionCall& operator=(FunctionCall const&) = delete;
  FunctionCall& operator=(FunctionCall &&) = delete;
  
  
};


class Builder::StructDefinition {
public:
  types::TypeHandle operator()(auto const&... args) && {
    static_assert(sizeof ... (args) % 2 == 0);
    std::vector<NameTypePair> fields;

    auto addField = [&]<typename ... Rest>(auto&& self, std::string const &name, types::TypeHandle type, Rest&& ... rest) -> void {
      fields.push_back(NameTypePair{name, type});
      if constexpr (sizeof ... (Rest) == 0) return;
      else self(self, std::forward<Rest>(rest)...);
    };

    addField(addField, std::forward<decltype(args)>(args)...);
    _called = true;
    return _builder.defineStructImpl(_structName, fields, API_FWD);
  }

  types::TypeHandle operator()(std::vector<NameTypePair> const &fields) && {
    _called = true;
    return _builder.defineStructImpl(_structName, fields, API_FWD);
  }

  
  ~StructDefinition() noexcept(false) {
    API_REQUIRE(_called, "operator() must be called on result of defineStruct(); e.g. defineStruct(\"Point\")(\"x\", i8, \"y\", i8);");
  }

private:
  friend class Builder;
  Builder& _builder;
  std::string _structName;
  api::Context API_CTX_NAME;
  bool _called = false;
  
  StructDefinition(Builder &b, std::string const &structName, api::Context const &ctx):
    _builder(b),
    _structName(structName),
    API_CTX_NAME(ctx)
  {}
  
  StructDefinition(StructDefinition const&) = delete;
  StructDefinition(StructDefinition&&) = delete;
  StructDefinition& operator=(StructDefinition const&) = delete;
  StructDefinition& operator=(StructDefinition&&) = delete;
    
};
  

#include "acus/builder/builder_private.tpp"
#include "acus/builder/builder_public.tpp"

} // namespace acus
