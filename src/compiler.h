#pragma once
#include <stack>
#include "program.h"
#include "function.h"
#include "data.h"

// ============================================================
// Compiler
// ============================================================

class Compiler {
  Program _program;
  Function* _currentFunction = nullptr;
  Function::Block* _currentBlock = nullptr;
  primitive::Sequence* _currentSeq = nullptr;
  bool _nextBlockIsSet = false;

  types::TypeSystem _ts;
  DataPointer _dp;

  struct MetaBlock {
    std::string name;
    std::string caller;
    std::string callee;
    std::string returnVar;
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
  inline types::TypeSystem &typeSystem() { return _ts; }
  std::string dumpPrimitives() const;
  std::string dumpBrainfuck() const;

  // IR Directives
  void setEntryPoint(std::string functionName);
  void begin();
  void end();
  void beginFunction(std::string const &name, FunctionSignature const &sig);
  void endFunction();
  void beginBlock(std::string name);
  void endBlock();
  void setNextBlock(int index);
  void setNextBlock(std::string f, std::string b = "");

  void returnFromFunction();
  void returnFromFunction(std::string const &var);
  void returnFromFunction(Slot const &slot);
  void returnFromFunction(values::Value const &value);
  
  void abortProgram();
  void referGlobals(std::vector<std::string> const &names);
  void callFunction(std::string const& functionName, std::string const& nextBlockName,
		    std::vector<values::Value> const &args,
		    std::string const &returnVar = "");
  
  inline void callFunction(std::string const& functionName, std::string const& nextBlockName, std::string const &returnVar = "") {
    callFunction(functionName, nextBlockName, {}, returnVar);
  }

  void assign(Slot const &dest, Slot const &src);
  void assign(Slot const &slot, std::string const &var);
  void assign(Slot const &slot, values::Value const &value);
  void assign(std::string const &var, Slot const &src);
  void assign(std::string const &var, values::Value const &value);
  void assign(std::string const &destVar, std::string const &srcVar);
    

  void writeOut(std::string const &var); 
  void writeOut(Slot const &slot);
  Slot arrayElementConst(std::string const &name, int index);
  Slot arrayElementConst(Slot const &slot, int index);

  Slot &declareLocal(std::string const &name, types::TypeHandle type);
  Slot &declareGlobal(std::string const &name, types::TypeHandle type);
  Slot &declareGlobalReference(Slot const &globalSlot);
  Slot &local(std::string const& name);
  Slot &global(std::string const& name);

  
  template <typename ... Args>
  void beginFunction(std::string const &name, types::TypeHandle returnType, Args&& ... args) {
    assert(_currentFunction == nullptr);

    beginFunction(name, FunctionSignature{
	returnType,
	std::forward<Args>(args)...
      });
  }

  inline void beginFunction(std::string const &name) {
    // convenience overload for void f(void)
    beginFunction(name, _ts.voidT());
  }
  
private:
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
  void copyArgsToNextFrame(std::string const &functionName, std::vector<values::Value> const &args);
  void markStartOfOriginFrame();  
  void moveToPreviousFrame();
  void moveToGlobalFrame(int payload = 0);  
  void moveToOriginFrame(int payload = 0);
  void fetchReturnData();
  void fetchReturnData(Slot const &returnSlot);

  // Global Data Synchronization (compiler_globals.cc)
  void fetchGlobal(Slot const &globalSlot, Slot const &localSlot);
  void putGlobal(Slot const &globalSlot, Slot const &localSlot);

  template <auto FetchOrPut>
  void syncGlobal(Slot const &localSlot) {
    assert(localSlot.storageType == Slot::GlobalReference);
    
    std::string globalName = localSlot.name.substr(std::string("__g_").size());
    assert(_program.globals.contains(globalName));

    Slot const &globalSlot = _program.globals.at(globalName);
    assert(globalSlot.size() == localSlot.size());

    (this->*FetchOrPut)(globalSlot, localSlot);
  }  

  template <auto FetchOrPut>
  void syncGlobals() {
    auto const &locals = _currentFunction->frame.locals;
    for (auto const &[localName, localSlot]: locals) {
      if (localSlot.storageType != Slot::GlobalReference) continue;
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
				  std::vector<values::Value> const &args);

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
  
};
