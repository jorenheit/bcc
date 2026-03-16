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

  DataPointer _dp;

  struct MetaBlock {
    std::string name;
    std::string caller;
    std::string nextBlockName;
  };
  std::vector<MetaBlock> _metaBlocks;

  struct PointerState {
    int offset;
    MacroCell::Field field;
  };
  std::stack<PointerState> _ptrStack;

public:
  std::string dumpPrimitives() const;
  std::string dumpBrainfuck() const;

  // IR Directives
  void setEntryPoint(std::string functionName);
  void begin();
  void end();
  void beginFunction(std::string name);
  void endFunction();
  void beginBlock(std::string name);
  void endBlock();
  void assignConst(int offset, int value);
  void writeOut(int offset, MacroCell::Field field = MacroCell::Value0);
  void setNextBlock(int index);
  void setNextBlock(std::string f, std::string b = "");
  void returnFromFunction();
  void abortProgram();
  void callFunction(std::string const& functionName, std::string const& nextBlockName);
  void referGlobals(std::vector<std::string> const &names);      

  Slot &declareLocal(std::string const& name, std::shared_ptr<types::Type>);
  Slot &declareGlobal(std::string const &name, std::shared_ptr<types::Type>);
  Slot &declareGlobalReference(Slot const &globalSlot);
  Slot &local(std::string const& name);
  Slot &global(std::string const& name);

  template <typename T, typename ... Args>
  Slot &declareLocal(std::string const& name, Args&& ... args) {
    auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
    return declareLocal(name, std::move(ptr));
  }

  template <typename T, typename ... Args>
  Slot &declareGlobal(std::string const& name, Args&& ... args) {
    auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
    return declareGlobal(name, std::move(ptr));
  }
  

private:
  // Memory management (compiler_memory.cc)
  // Slot &allocateTemp(int size = 1);
  // void freeTemp(Slot &slot);
  
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
  void markStartOfOriginFrame();  
  void moveToPreviousFrame();
  void moveToGlobalFrame(int payload = 0);  
  void moveToOriginFrame(int payload = 0);
  void fetchReturnData();

  // Global Data (compiler_globals.cc)
  void fetchGlobal(Slot const &globalSlot, Slot const &localSlot);
  void putGlobal(Slot const &globalSlot, Slot const &localSlot);
  void syncGlobalToLocal();
  void syncLocalToGlobal();
  
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
