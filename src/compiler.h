#pragma once
#include <stack>
#include <sstream>
#include <optional>
#include "program.h"
#include "function.h"
#include "data.h"
#include "rlvalue.h"
#include "types.h"

// ============================================================
// Compiler
// ============================================================

class Compiler {

  Program _program;
  Function* _currentFunction = nullptr;
  Function::Block* _currentBlock = nullptr;
  Function::Scope* _currentScope = nullptr;
  primitive::Sequence* _currentSeq = nullptr;  
  bool _nextBlockIsSet = false;

  struct {
    bool begun = false;
    bool allowGlobalDefinitions = true;
  } _state;
  
  DataPointer _dp;

  struct MetaBlock {
    std::string name;
    std::string caller;
    std::string callee;
    std::optional<Slot> returnSlot;
    std::string nextBlockName;
  };
  std::vector<MetaBlock> _metaBlocks;

  struct PointerState {
    int offset;
    MacroCell::Field field;
  };
  std::stack<PointerState> _ptrStack;

  struct FunctionCall {
    std::string caller, callee;
    std::vector<types::TypeHandle> args;
  };
  std::vector<FunctionCall> _deferredFunctionCallTypeChecks;


public:
  Compiler() { TypeSystem::init(); }
  
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
  Slot getStructField(L const &obj, std::string const &field) {
    return getStructFieldImpl(lValue(obj), field);
  }

  template <typename R>
  Slot arrayElementConst(R const &obj, int index) {
    return arrayElementConstImpl(lValue(obj), index);
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
  values::RValue rValue(Slot const &slot)          const { return values::RValue{slot};  }
  values::RValue rValue(values::Value const &val)  const { return (val->isRef() ? rValue(val->varName()) : values::RValue{val});  }
  values::RValue rValue(values::Ref const &var)    const { return rValue(var->varName());  }

  values::LValue lValue(values::LValue const &val) const { return values::LValue{val}; }
  values::LValue lValue(std::string const &var)    const { return values::LValue{local(var)};  }
  values::LValue lValue(Slot const &slot)          const { return values::LValue{slot};  }
  values::LValue lValue(values::Ref const &var)    const { return values::LValue{local(var->varName())};  }

  // Implementation functions for public interface
  void callFunctionImpl(std::string const& functionName, std::string const& nextBlockName,
			std::optional<values::LValue> const &returnSlot, std::vector<values::RValue> const &args);
  void returnFromFunctionImpl(std::optional<values::RValue> const &ret = {});
  Slot getStructFieldImpl(values::LValue const &obj, std::string const &field);
  Slot arrayElementConstImpl(values::LValue const &arr, int index);

  void assignImpl(values::LValue const &lhs, values::RValue const &rhs);
  void writeOutImpl(values::RValue const &rhs); 
  
  
  // Algorithms: all applied to the current DP (compiler_algorithms.cc)
  void moveTo(int frameOffset);  
  void moveTo(int frameOffset, MacroCell::Field field);
  void moveToOrigin();
  void switchField(MacroCell::Field field);  
  void zeroCell();
  void addConst(int delta);
  void subConst(int delta);
  void setToValue(uint8_t value);
  void notValue();
  void notValue(MacroCell::Field resultField);
  void cmpConst(int value, MacroCell::Field resultField);
  void moveField(int destOffset, MacroCell::Field destField);
  void moveField(int destOffset1, MacroCell::Field destField1, int destOffset2, MacroCell::Field destField2);
  void copyField(int destOffset, MacroCell::Field destField);
  void loopOpen(std::string const &tag = defaultOpenTag());
  void loopClose(std::string const &tag = defaultCloseTag());

  // Frame Navigation (compiler_framenav.cc)
  void pushFrame();
  void popFrame();
  void seek(primitive::Direction dir, int payload, bool skipFirstCheck = true);
  void copyArgsToNextFrame(std::string const &functionName, std::vector<values::RValue> const &args);
  void setSeekMarker();
  void markStartOfOriginFrame();  
  void moveToPreviousFrame();
  void moveToGlobalFrame(int payload = 0);  
  void moveToOriginFrame(int payload = 0);
  void fetchReturnData();
  void fetchReturnData(Slot const &returnSlot);

  // Temporaries and memory management (compiler_memory.cc)
  void freeSlot(Slot &slot);
  void freeTemps();
  void freeScope(Function::Scope const *scope);
  Slot allocSlot(std::string const &name, types::TypeHandle type, Slot::Kind kind);
  Slot getTemp(types::TypeHandle type);
  Slot getTemp(values::Value const &val);
  
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

  template <typename... Args>  requires ((std::convertible_to<Args, int>) && ...)
  auto getFieldIndices(Args... args)  {
    static_assert(sizeof...(Args) % 2 == 0, "getFieldIndices requires an even number of arguments");
    constexpr std::size_t pairCount = sizeof...(Args) / 2;    
    auto allArgs = std::make_tuple(static_cast<int>(args)...);
    return [&]<std::size_t... I>(std::index_sequence<I...>) {
      return std::make_tuple(getFieldIndex(std::get<2 * I>(allArgs), std::get<2 * I + 1>(allArgs))...);
    }(std::make_index_sequence<pairCount>{});
  }

  template <typename... Fields>
  int pickScratchField(Fields... fields) {
    static_assert((std::convertible_to<Fields, int> && ...));
    const bool scratch0Used = ((fields == MacroCell::Scratch0) || ...);
    const bool scratch1Used = ((fields == MacroCell::Scratch1) || ...);
    assert(!(scratch0Used && scratch1Used));
    return scratch0Used ? MacroCell::Scratch1 : MacroCell::Scratch0;
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
