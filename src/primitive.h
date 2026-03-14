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
    inline explicit MovePointerRelative(DInt amount): amount(std::move(amount)) {}
    COMMON_INTERFACE;
  };

  enum Direction { Left, Right };
  
  struct MovePointerDynamic: Node {
    Direction dir;
    int stride, value, flag, tmp1, tmp2;
    inline explicit MovePointerDynamic(Direction d, int stride, int value, int flag, int tmp1, int tmp2):
      dir(d),
      stride(stride),
      value(value),
      flag(flag),
      tmp1(tmp1),
      tmp2(tmp2)
    {}
    
    COMMON_INTERFACE;
  };

  
  struct ZeroCell: Node {
    COMMON_INTERFACE;
  };

  struct ChangeBy: Node {
    DInt delta;
    inline explicit ChangeBy(DInt d): delta(std::move(d)) {}
    COMMON_INTERFACE;
  };
  
  struct MoveData: Node {
    DInt current, dest;

    inline explicit MoveData(DInt diff):
      current(0),
      dest(diff)
    {}
    
    inline explicit MoveData(DInt current, DInt dest):
      current(std::move(current)),
      dest(std::move(dest))
    {}
    
    COMMON_INTERFACE;
  };
  
  struct CopyData: Node {
    DInt current, dest, scratch;
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


} // namespace ir
