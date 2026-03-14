#pragma once
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
  

public:
  // TODO: distinguish between public and private interface
  
  inline Program const& program() const { return _program; }
  inline Program& program() { return _program; }
  inline void setTargetSequence(primitive::Sequence *seq) { _currentSeq = seq; }
  
  void begin();
  void end();
  
  void beginFunction(std::string name);
  void endFunction();
  void setEntryPoint(std::string functionName);

  void beginBlock(std::string name);
  void endBlock();

  void blockOpen();
  void blockClose();
  void constructMetaBlocks();
  
  void moveToOrigin();
  void resetOrigin();
  
  template <typename Primitive, typename ... Args>
  void emit(Args&& ... args) {
    assert(_currentSeq != nullptr);
    _currentSeq->emplace<Primitive>(std::forward<Args>(args)...);
  }

  void pushFrame();
  void setFrameMarker(MacroCell::FrameMarkerValue value);
  
  void popFrame();
  void fetchReturnData();
  void abortProgram();
  
  void setNextBlock(int index);
  void setNextBlock(std::string f, std::string b = "");
  void switchField(MacroCell::Field field);

  static inline std::string defaultOpenTag() {
    static int count = 0;
    return std::string("open_loop_") + std::to_string(count++);
  } 
  static inline std::string defaultCloseTag() {
    static int count = 0;
    return std::string("close_loop_") + std::to_string(count++);
  }
 
  void loopOpen(std::string const &tag = defaultOpenTag());
  void loopOpen(int frameOffset, std::string const &tag = defaultOpenTag());
  void loopClose(std::string const &tag = defaultCloseTag());
  void loopClose(int frameOffset, std::string const &tag = defaultCloseTag());

  void moveTo(int frameOffset = -1);  
  void zeroCell(int frameOffset = -1);
  void addConst(int delta, int frameOffset = -1);
  void subConst(int delta, int frameOffset = -1);
  void setToValue(uint8_t value, int frameOffset = -1);
  void writeOut(int frameOffset = -1);
  
  void returnFromFunction();
  void syncGlobalToLocal();
  void syncLocalToGlobal();

  void callFunction(std::string const& functionName, std::string const& nextBlockName);
    
  Slot &declareLocal(std::string const& name, int size = 1);
  Slot &declareGlobal(std::string const &name, int size = 1);
  void referGlobals(std::vector<std::string> const &names);    

  Slot &local(std::string const& name);
  Slot &global(std::string const& name);    
  Slot &allocateTemp(int size = 1);
  void freeTemp(Slot &slot);
  
  primitive::Context constructContext() const;    
  primitive::Sequence compilePrimitives() const;
  std::string dumpPrimitives() const;
  std::string dumpBrainfuck() const;
  
  static std::string simplifyProgram(std::string const &bf);
};
