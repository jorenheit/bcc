#pragma once
#include <iostream>
#include <cassert>
#include <vector>
#include <memory>
#include <utility>
#include "defer.h"

namespace primitive {

  
  struct Context {
    
    int fieldCount;
    std::unordered_map<std::string, int> blockIDtoIndex;
    std::unordered_map<std::string, int> stackFrameSize;

    int getBlockIndex(std::string const &f, std::string const &b = "") const {      
      std::string const id = f + (b.empty() ? "" : (std::string(".") + b));
      return blockIDtoIndex.at(id);
    }

    int getStackFrameSize(std::string const &f) const {
      return stackFrameSize.at(f);
    }
  };

  using DInt = Defer::Int<Context>;
  
  struct Node {
    virtual ~Node() = default;
    virtual std::string text(Context const&) const = 0;
    virtual std::string generate(Context const&) const = 0;
  };

  struct Sequence {
    std::vector<std::shared_ptr<Node>> nodes;

    template <typename T, typename... Args>
    void emplace(Args&&... args) {
      nodes.push_back(std::make_shared<T>(std::forward<Args>(args)...));
    }

    inline void append(Sequence const &other) {
      for (auto const &n : other.nodes) {
	nodes.push_back(n);
      }
    }

    std::string dumpText(Context const &ctx);
    std::string dumpCode(Context const &ctx);
  };


  enum Direction { Left, Right };
  
  // ========= Primitive Nodes =============

#define COMMON_INTERFACE				 \
  std::string text(Context const&) const override;	 \
  std::string generate(Context const&) const override;	 \


  struct LoopOpen: Node {
    std::string tag;
    inline explicit LoopOpen(std::string tag): tag(std::move(tag)) {}
    COMMON_INTERFACE;
  };

  struct LoopClose: Node {
    std::string tag;
    inline explicit LoopClose(std::string tag): tag(std::move(tag)) {}
    COMMON_INTERFACE;
  };

  struct MovePointerRelative: Node {
    DInt amount = 0;

    /*
      Moves the pointer by a fixed amount.
      .
      Assumed initial pointer position: -
      Assumed empty: -
      Invariants: -
     */
    
    inline explicit MovePointerRelative(DInt amount): amount(std::move(amount)) {}
    COMMON_INTERFACE;
  };

  
  struct ZeroCell: Node {

    /*
      Sets the value of the current cell to 0.
      Assumed initial pointer position: -
      Assumed empty: -      
      Invariants: -
     */
    
    COMMON_INTERFACE;
  };

  struct ZeroCellPlus: Node {

    /*
      Sets the value of the current cell to 0 by incrementing it.
      Assumed initial pointer position: -
      Assumed empty: -      
      Invariants: -
     */
    
    COMMON_INTERFACE;
  };

  
  struct ChangeBy: Node {
    DInt delta;
    /*
      Changes the value in the current cell by an amount 'delta'.
      
      Assumed initial pointer position: -
      Assumed empty: -      
      Invariants: -
    */
    
    inline explicit ChangeBy(DInt d): delta(std::move(d)) {}
    COMMON_INTERFACE;
  };
  
  struct MoveData: Node {
    DInt current, dest;

    /*
      Moves the value stored in 'current' into 'dest'.
      This leaves 'current' empty!
      
      Assumed initial pointer position: current
      Assumed empty: -      
      Invariants: ptr
     */
    
    inline explicit MoveData(DInt diff):
      current(0),
      dest(std::move(diff))
    {}
    
    inline explicit MoveData(DInt current, DInt dest):
      current(std::move(current)),
      dest(std::move(dest))
    {}

    COMMON_INTERFACE;
  };

  struct MoveData2: Node {
    DInt current, dest1, dest2;

    /*
      Moves the value stored in 'current' into both 'dest1' and 'dest2'.
      This leaves 'current' empty!
      
      Assumed initial pointer position: current
      Assumed empty: -      
      Invariants: ptr
     */
    
    inline explicit MoveData2(DInt diff1, DInt diff2):
      current(0),
      dest1(std::move(diff1)),
      dest2(std::move(diff2))
    {}
    
    inline explicit MoveData2(DInt current, DInt dest1, DInt dest2):
      current(std::move(current)),
      dest1(std::move(dest1)),
      dest2(std::move(dest2))
    {}

    COMMON_INTERFACE;
  };
  
  struct CopyData: Node {
    DInt current, dest, scratch;

    /*
      Copies the value stored in 'current' into 'dest'.
      
      Assumed initial pointer position: current
      Assumed empty: scratch
      Invariants: ptr, scratch
     */
    
    inline explicit CopyData(DInt current, DInt dest, DInt scratch):
      current(std::move(current)),
      dest(std::move(dest)),
      scratch(std::move(scratch))
    {}

    COMMON_INTERFACE;
  };
  
  struct Out: Node {
    COMMON_INTERFACE;
  };


  struct Not1: Node {
    DInt current, scratch;

    /*
      Not operator applied to the value stored at 'current'. Overwrites
      'current' with 0 if nonzero, 1 if zero (destructive).

      Assumed initial pointer position: current
      Assumed empty: scratch
      Invariants: ptr, scratch
     */
    
    inline explicit Not1(DInt current, DInt scratch):
      current(std::move(current)),
      scratch(std::move(scratch))
    {}

    COMMON_INTERFACE;
  };
  
  struct Not2: Node {
    DInt current, result, scratch1, scratch2;

    /*
      Not operator applied to the value stored at 'current'. Stores 1 in 'result'
      if the value is zero and 0 if the value is nonzero.
      
      Assumed initial pointer position: current
      Assumed empty: result, scratch1, scratch2      
      Invariants: ptr, scratch1, scratch2
     */
    
    inline explicit Not2(DInt current, DInt result, DInt scratch1, DInt scratch2):
      current(std::move(current)),
      result(std::move(result)),
      scratch1(std::move(scratch1)),
      scratch2(std::move(scratch2))
    {}

    COMMON_INTERFACE;
  };

  struct Cmp: Node {
    DInt value, current, result, scratch1, scratch2;
    
    /*
      Compares the value stored at 'current' to 'value'. Stores 1 in 'result'
      if equal, 0 otherwise.
      
      Assumed initial pointer position: current
      Assumed empty: result, scratch1, scratch2
      Invariants: ptr, scratch1, scratch2
     */
    
    inline explicit Cmp(DInt value, DInt current, DInt result, DInt scratch1, DInt scratch2):
      value(std::move(value)),
      current(std::move(current)),
      result(std::move(result)),
      scratch1(std::move(scratch1)),
      scratch2(std::move(scratch2))
    {}

    COMMON_INTERFACE;
  };
  
} // namespace ir
