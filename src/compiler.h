#pragma once
#include <stack>
#include <sstream>
#include <optional>
#include "proxy.h"
#include "program.h"
#include "function.h"
#include "data.h"
#include "rlvalue.h"
#include "types.h"

// ============================================================
// Compiler
// ============================================================

class Compiler {
  //  friend class proxy::Impl::Base;
  friend class proxy::Impl::Direct;
  friend class proxy::Impl::ArrayElement;
  friend class proxy::Impl::StructField;

  
  Program _program;
  Function* _currentFunction = nullptr;
  Function::Block* _currentBlock = nullptr;
  Function::Scope* _currentScope = nullptr;
  primitive::Sequence* _currentSeq = nullptr;  
  bool _nextBlockIsSet = false;

  struct {
    bool begun = false;
    bool allowGlobalDefinitions = true;
    bool seekMarkerSet = false;
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
    std::string caller, callee;
    std::vector<types::TypeHandle> args;
  };
  std::vector<FunctionCall> _deferredFunctionCallTypeChecks;

public:
  inline Compiler() { TypeSystem::init(); }
  
  std::string dumpPrimitives() const;
  std::string dumpBrainfuck() const;

  // IR Directives
  void setEntryPoint(std::string functionName);
  void begin();
  void end();
  void beginFunction(std::string const &name, FunctionSignature const &sig);
  void endFunction();
  void beginScope();
  void endScope();
  void beginBlock(std::string name);
  void endBlock();
  void setNextBlock(int index);
  void setNextBlock(std::string f, std::string b);
  void abortProgram();
  void referGlobals(std::vector<std::string> const &names);

  template <typename ... Args> 
  types::TypeHandle defineStruct(std::string const &name, Args&& ... args){
    types::TypeHandle s = TypeSystem::defineStruct(name, std::forward<Args>(args)...);
    error_if(s == nullptr, "conflicting definitions of struct '", name, "'.");
    return s;
  }

  template <typename... Args>
  void callFunction(std::string const& functionName, std::string const& nextBlockName, Args&&... args) {
    callFunctionImpl(functionName, nextBlockName, std::optional<values::LValue>{},
		     constructArgumentList(std::forward<Args>(args)...));
  }

  template <typename L, typename... Args>
  void callFunctionReturn(std::string const& functionName, std::string const& nextBlockName, L&& returnSlot, Args&&... args) {
    callFunctionImpl(functionName, nextBlockName,
		     std::make_optional<values::LValue>(lValue(std::forward<L>(returnSlot))),
		     constructArgumentList(std::forward<Args>(args)...));
  }

  void callFunction(std::string const& functionName, std::string const& nextBlockName) {
    callFunctionImpl(functionName, nextBlockName, {}, {});
  }
  

  template <typename R>
  void returnFromFunction(R const &rhs) {
    returnFromFunctionImpl(std::make_optional<values::RValue>(rValue(rhs)));
  }

  void returnFromFunction() {
    returnFromFunctionImpl({});
  }

  template <typename L, typename R>
  void assign(L const &lhs, R const &rhs) {
    assignImpl(lValue(lhs), rValue(rhs));
  }

  template <typename R>
  void writeOut(R const &rhs) {
    writeOutImpl(rValue(rhs));
  }

  template <typename L>
  SlotProxy structField(L const &obj, std::string const &field) {
    return structFieldImpl(lValue(obj), field);
  }

  template <typename L>
  SlotProxy structField(L const &obj, int fieldIndex) {
    return structFieldImpl(lValue(obj), fieldIndex);
  }

  template <typename Array>
  SlotProxy arrayElement(Array const &arr, int index) {
    return arrayElementImpl(lValue(arr), index);
  }
  
  template <typename Array, typename Index>
  SlotProxy arrayElement(Array const &arr, Index const &index) {
    return arrayElementImpl(lValue(arr), rValue(index));
  }
  
  Slot declareLocal(std::string const &name, types::TypeHandle type);
  Slot declareGlobal(std::string const &name, types::TypeHandle type);
  Slot declareGlobalReference(Slot const &globalSlot);

  // TODO: rename to resolveVar(...)
  Slot local(std::string const& name, bool globalReference = false) const;

  template <typename ... Args>
  void beginFunction(std::string const &name, types::TypeHandle returnType, Args&& ... args) {
    beginFunction(name, FunctionSignature{
	returnType,
	std::forward<Args>(args)...
      });
  }

  inline void beginFunction(std::string const &name) {
    beginFunction(name, TypeSystem::voidT());
  }

  
private:

  
  
  // Normalize to RValue or LValue
  values::RValue rValue(values::RValue const &val) const { return values::RValue{val}; }
  values::RValue rValue(std::string const &var)    const { return values::RValue{local(var)};  }
  values::RValue rValue(SlotProxy const &slot)          const { return values::RValue{slot};  }
  values::RValue rValue(values::Anonymous const &val)  const { return (val->isRef() ? rValue(val->varName()) : values::RValue{val});  }

  values::LValue lValue(values::LValue const &val) const { return values::LValue{val}; }
  values::LValue lValue(std::string const &var)    const { return values::LValue{local(var)};  }
  values::LValue lValue(SlotProxy const &slot)          const { return values::LValue{slot};  }

  // Implementation functions for public interface
  void callFunctionImpl(std::string const& functionName, std::string const& nextBlockName,
			std::optional<values::LValue> const &returnSlot, std::vector<values::RValue> const &args);
  void returnFromFunctionImpl(std::optional<values::RValue> const &ret = {});
  SlotProxy structFieldImpl(values::LValue const &obj, std::string const &field);
  SlotProxy structFieldImpl(values::LValue const &obj, int fieldIndex);
  SlotProxy arrayElementImpl(values::LValue const &arr, int index);
  SlotProxy arrayElementImpl(values::LValue const &arr, values::RValue const &index);
  SlotProxy deref(values::RValue const &ptr);

  void assignImpl(values::LValue const &lhs, values::RValue const &rhs);
  void writeOutImpl(values::RValue const &rhs); 
  
  // Slot operations
  void assignSlot(Slot const &dest, Slot const &src);
  void assignSlot(Slot const &slot, values::Anonymous const &val);
  void copySlotIntoElement(Slot const &srcSlot, Slot const &arrSlot, Slot const &indexSlot);
  void copyElementIntoSlot(Slot const &elementSlot, Slot const &arrSlot, Slot const &indexSlot);
  
  // Algorithms: all applied to the current DP (compiler_algorithms.cc)
  void moveTo(Cell cell);
  void moveTo(int offset, MacroCell::Field field = MacroCell::Value0);
  void moveToOrigin();
  void moveRel(int diff);
  void switchField(MacroCell::Field field);  
  void zeroCell();
  void loopOpen(std::string const &tag = defaultOpenTag());
  void loopClose(std::string const &tag = defaultCloseTag());

  void goToDynamicOffset(Cell offsetLow, Cell offsetHigh);
  void fetchFromDynamicOffset(Cell offsetLow, Cell offsetHigh, Payload const &payload, primitive::Direction seekDir);
  
  void moveField(Cell dest);
  void copyField(Cell dest, Temps<1> tmp);
  
  void setToValue(int value);
  void setToValue16(int value, Cell high);

  void inc();
  void dec();
  void inc16(Cell high, Temps<2> tmp);
  void dec16(Cell high, Temps<2> tmp);
  
  void addConst(int delta);
  void addConstAndCarry(int delta, Cell carry, Temps<3> tmp);
  void add16Const(int delta, Cell high, Temps<4> tmp);

  void subConst(int delta);
  void subConstAndCarry(int delta, Cell carry, Temps<3> tmp);
  void sub16Const(int delta, Cell high, Temps<4> tmp);

  void mulConst(int factor, Temps<3> tmp);
  //  void mul16Const(int factor, Cell high, Temps<?> tmp);

  
  // TODO: constructive versions should accept "other" before result and carry
  void addDestructive(Cell other);
  void addConstructive(Cell result, Cell other, Temps<2> tmp);
  void add16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<3> tmp);
  void add16Constructive(Cell high, Cell resultLow, Cell resultHigh, Cell otherLow, Cell otherHigh, Temps<5> tmp);
  void addAndCarryDestructive(Cell carry, Cell other, Temps<2> tmp);
  void addAndCarryConstructive(Cell result, Cell carry, Cell other, Temps<3> tmp);

  void subDestructive(Cell other);
  void subConstructive(Cell result, Cell other, Temps<2> tmp);
  void sub16Destructive(Cell high, Cell otherLow, Cell otherHigh, Temps<3> tmp);
  void sub16Constructive(Cell high, Cell resultLow, Cell resultHigh, Cell otherLow, Cell otherHigh, Temps<5> tmp);
  void subAndCarryDestructive(Cell carry, Cell other, Temps<2> tmp);
  void subAndCarryConstructive(Cell result, Cell carry, Cell other, Temps<3> tmp);

  void notDestructive(Temps<1> tmp);
  void notConstructive(Cell result, Temps<1> tmp);

  void orDestructive(Cell other, Temps<1> tmp);
  void orConstructive(Cell result, Cell other, Temps<2> tmp);

  void andDestructive(Cell other, Temps<1> tmp);
  void andConstructive(Cell result, Cell other, Temps<2> tmp);

  void compareToConstDestructive(int value, Temps<1> tmp);
  void compareToConstConstructive(int value, Cell result, Temps<1> tmp);    

  void compare16ToConstDestructive(int value, Cell high, Temps<1> tmp);
  void compare16ToConstConstructive(int value, Cell high, Cell result, Temps<2> tmp);    
  
  void lessDestructive(Cell other, Temps<2> tmp);
  void lessConstructive(Cell result, Cell other, Temps<3> tmp);

  void lessOrEqualDestructive(Cell other, Temps<2> tmp);
  void lessOrEqualConstructive(Cell result, Cell other, Temps<3> tmp);

  void greaterDestructive(Cell other, Temps<2> tmp);
  void greaterConstructive(Cell result, Cell other, Temps<3> tmp);

  void greaterOrEqualDestructive(Cell other, Temps<2> tmp);
  void greaterOrEqualConstructive(Cell result, Cell other, Temps<3> tmp);

  void equalDestructive(Cell other, Temps<2> tmp);
  void equalConstructive(Cell result, Cell other, Temps<3> tmp);

  
  // Frame Navigation (compiler_framenav.cc)
  
  void pushFrame();
  void popFrame();
  void seek(MacroCell::Field markerField, primitive::Direction dir, Payload const &payload, bool checkCurrent);
  void setSeekMarker();
  void resetSeekMarker();
  void moveToPreviousFrame(Payload const &payload = {});  
  void initializeArguments(std::string const &functionName, std::vector<values::RValue> const &args);  
  void fetchReturnData();
  void fetchReturnData(Slot const &returnSlot);

  // Temporaries and memory management (compiler_memory.cc)
  void freeSlot(Slot &slot);
  void freeTemps();
  void freeScope(Function::Scope const *scope);
  Slot allocSlot(std::string const &name, types::TypeHandle type, Slot::Kind kind);
  Slot getTemp(types::TypeHandle type);
  Slot getTemp(values::Anonymous const &val);
  
  // Global Data Synchronization (compiler_globals.cc)
  void fetchGlobal(Slot const &globalSlot, Slot const &localSlot);
  void putGlobal(Slot const &globalSlot, Slot const &localSlot);

  template <auto FetchOrPut>
  void syncGlobal(Slot const &localSlot) {
    assert(localSlot.kind == Slot::GlobalReference);
    
    std::string globalName = localSlot.name.substr(std::string("__g_").size());
    assert(_program.isGlobal(globalName));

    Slot const &globalSlot = _program.globalSlot(globalName);
    assert(globalSlot.size() == localSlot.size());

    (this->*FetchOrPut)(globalSlot, localSlot);
  }  

  template <auto FetchOrPut>
  void syncGlobals() {
    auto const &locals = _currentFunction->frame.locals;
    for (auto const &localSlot: locals) {
      if (localSlot.kind != Slot::GlobalReference) continue;
      syncGlobal<FetchOrPut>(localSlot);
    }  
  }

  void syncGlobalToLocal() {
    syncGlobals<&Compiler::fetchGlobal>();
  }

  void syncLocalToGlobal() {
    syncGlobals<&Compiler::putGlobal>();
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
				  std::vector<values::RValue> const &args);

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

  // Helper for creating argument vector
  template <typename ... Args> 
  std::vector<values::RValue> constructArgumentList(Args&& ... args) {
    std::vector<values::RValue> result; result.reserve(sizeof...(args));
    (result.emplace_back(rValue(std::forward<Args>(args))), ...);
    return result;
  }
  
  
  // Error Handling
  struct Error: std::exception {

    std::string msg;
    Error(std::string const &msgHead): msg(msgHead) {}
    
    template <typename T>
    Error &operator<<(T const &val) {
      std::ostringstream oss;
      oss << val;
      msg += oss.str();
      return *this;
    }

    virtual char const *what() const noexcept override {
      return msg.c_str();
    }
  };

  template <typename ... Args> requires (sizeof...(Args) > 0)
  void error(Args&& ... args) const {
    Error err("Backend error: "); (err << ... << args);
    throw err;
  }

  template <typename ... Args> requires (sizeof...(Args) > 0)
  void warning(Args&& ... args) const {
    Error err("Backend warning: "); (err << ... << args);
    throw err;
  }
  
  template <typename ... Args> requires (sizeof...(Args) > 0)
  void error_if(bool const condition, Args&& ... args) const {
    if (condition) error(std::forward<Args>(args)...);
  }

  template <typename ... Args> requires (sizeof...(Args) > 0)
  void warning_if(bool const condition, Args&& ... args) const {
    if (condition) warning(std::forward<Args>(args)...);
  }

  
  
};
