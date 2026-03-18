#include <sstream>
#include <iostream>
#include "primitive.h"
#include "data.h"

std::string primitive::Sequence::dumpText(Context const &ctx) {
  std::ostringstream oss;
  for (auto const &node: nodes) oss << node->text(ctx) << '\n';
  return oss.str();
}


std::string primitive::Sequence::dumpCode(Context const &ctx) {
  std::ostringstream oss;
  for (auto const &node: nodes) oss << node->generate(ctx);
  return oss.str();  
}


namespace Algorithm {
  
  // Each algorithm leaves the pointer in the cell indicated by
  // its first argument. All algorithms except movePtr take
  // the current cell as their first arg, so those algorithms
  // leave the pointer invariant.

  std::string movePtr(int amount) {
    char const ch = (amount > 0) ? '>' : '<';
    return std::string(std::abs(amount), ch);
  }
  
  std::string movePtr(int dest, int current) {
    return movePtr(dest - current);
  }

  std::string decrement(int n = 1) { assert(n >= 0); return std::string(n, '-'); }
  std::string increment(int n = 1) { assert(n >= 0); return std::string(n, '+'); }
  std::string modify(int n) {
    return (n > 0) ? increment(std::abs(n)) : decrement(std::abs(n));
  }

  std::string zero()      { return "[-]"; }
  std::string zeroPlus()  { return "[+]"; }
  
  std::string moveValue(int current, int target) {
    // [->+<]
    std::ostringstream oss;
    oss << "["
	<<   decrement()
	<<   movePtr(target, current)
	<<   increment()
	<<   movePtr(current, target)
	<< "]";
    return oss.str();
  }

  std::string moveValue(int current, int target1, int target2) {
    // [->+>+<<]
    std::ostringstream oss;
    oss << "["
	<<   decrement()
	<<   movePtr(target1, current)
	<<   increment()
	<<   movePtr(target2, target1)
	<<   increment()
	<<   movePtr(current, target2)
	<< "]";
    return oss.str();
  }

  std::string copyValue(int current, int target, int tmp) {
    // [->+>+<<]>>[-<<+>>]<<
    std::ostringstream oss;
    oss << moveValue(current, target, tmp)
	<< movePtr(tmp, current)
	<< moveValue(tmp, current)
	<< movePtr(current, tmp);
    return oss.str();
  }

  std::string notValue(int current, int result, int copy, int tmp) {
    // Store !current in result
    // assumes copy and tmp are 0
    // leaves copy and tmp 0
    
    std::ostringstream oss;
    oss << movePtr(result, current)
	<< increment()
	<< movePtr(current, result)
	<< copyValue(current, copy, tmp)
	<< movePtr(copy, current)
	<< "["
	<<   zero()
	<<   movePtr(result, copy)
	<<   decrement()
	<<   movePtr(copy, result)
	<< "]"
	<< movePtr(current, copy);

    return oss.str();
  }

  std::string notValue(int current, int tmp) {
    // Store !current back into current
    // assumes tmp = 0
    
    std::ostringstream oss;
    oss << movePtr(tmp, current)
	<< increment()
	<< movePtr(current, tmp)
	<< "["
	<<   zero()
	<<   movePtr(tmp, current)
	<<   decrement()
	<<   movePtr(current, tmp)
	<< "]"
	<< movePtr(tmp, current)
	<< "["
	<<   decrement()
	<<   movePtr(current, tmp)
	<<   increment()
	<<   movePtr(tmp, current)
	<< "]"
	<< movePtr(current, tmp);

    return oss.str();
  }
  
  std::string cmpConst(int value, int current, int result, int copy, int tmp) {

    std::ostringstream oss;
    oss << movePtr(result, current)
	<< increment()
	<< movePtr(current, result)
	<< copyValue(current, copy, tmp)
	<< movePtr(copy, current)
	<< decrement(value)
	<< "["
	<<   zero()
	<<   movePtr(result, copy)
	<<   decrement()
	<<   movePtr(copy, result)
	<< "]"
	<< movePtr(current, copy);
    return oss.str();
  }  
}


#define TXT(Name) std::string primitive::Name::text(Context const &ctx) const 
#define GEN(Name) std::string primitive::Name::generate(Context const &ctx) const

// LoopOpen
TXT(LoopOpen) { return std::string("loop open; tag = ") + tag; }
GEN(LoopOpen) { return "["; }

// LoopClose
TXT(LoopClose) { return std::string("loop close; tag = ") + tag; }
GEN(LoopClose) { return "]"; }

// MovePointerRelative
TXT(MovePointerRelative) {
  return std::string("move_ptr_rel ") + std::to_string(amount.resolve(ctx));
}

GEN(MovePointerRelative) {
  return Algorithm::movePtr(amount.resolve(ctx));
}

// ZeroCell
TXT(ZeroCell) { return "zero_cell"; }
GEN(ZeroCell) { return Algorithm::zero(); }

// ZeroCellPlus
TXT(ZeroCellPlus) { return "zero_cell"; }
GEN(ZeroCellPlus) { return Algorithm::zeroPlus(); }

// ChangeBy
TXT(ChangeBy) { return std::string("change_by ") + std::to_string(delta.resolve(ctx)); }
GEN(ChangeBy) { return Algorithm::modify(delta.resolve(ctx)); }

// MoveData
TXT(MoveData) { return "move_data"; } // TODO add indices
GEN(MoveData) {
  int const cur = current.resolve(ctx);
  int const dst = dest.resolve(ctx);
  return Algorithm::moveValue(cur, dst);
}

// MoveData2
TXT(MoveData2) { return "move_data2"; } // TODO add indices
GEN(MoveData2) {
  int const cur = current.resolve(ctx);
  int const dst1 = dest1.resolve(ctx);
  int const dst2 = dest2.resolve(ctx);
  return Algorithm::moveValue(cur, dst1, dst2);
}

// CopyData
TXT(CopyData) { return "copy_data"; } // TODO add indices
GEN(CopyData) {
  int const cur = current.resolve(ctx);
  int const dst = dest.resolve(ctx);
  int const tmp = scratch.resolve(ctx);
  return Algorithm::copyValue(cur, dst, tmp);
}


// Not (destructive version)
TXT(Not1) { return "not1"; } 
GEN(Not1) {
  int const cur = current.resolve(ctx);
  int const tmp = scratch.resolve(ctx);
  return Algorithm::notValue(cur, tmp);
}

// Not (non-destructive version)
TXT(Not2) { return "not2"; } 
GEN(Not2) {
  int const cur = current.resolve(ctx);
  int const res = result.resolve(ctx);
  int const tmp1 = scratch1.resolve(ctx);
  int const tmp2 = scratch2.resolve(ctx);
  return Algorithm::notValue(cur, res, tmp1, tmp2);
}


// Cmp
TXT(Cmp) { return "cmp"; } 
GEN(Cmp) {
  int const val = value.resolve(ctx);
  int const cur = current.resolve(ctx);
  int const res = result.resolve(ctx);
  int const tmp1 = scratch1.resolve(ctx);
  int const tmp2 = scratch2.resolve(ctx);
  return Algorithm::cmpConst(val, cur, res, tmp1, tmp2);
}

// Out
TXT(Out) { return "out"; }
GEN(Out) { return "."; }
