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

  std::string decrement(int n) { assert(n >= 0); return std::string(n, '-'); }
  std::string increment(int n) { assert(n >= 0); return std::string(n, '+'); }
  std::string modify(int n) {
    return (n > 0) ? increment(std::abs(n)) : decrement(std::abs(n));
  }

  std::string zero()      { return "[-]"; }
  std::string zeroPlus()  { return "[+]"; }
  
  std::string moveValue(int current, int target) {
    // [->+<]
    std::ostringstream oss;
    oss << "["
	<< decrement(1)
	<< movePtr(target, current)
	<< increment(1)
	<< movePtr(current, target)
	<< "]";
    return oss.str();
  }

  std::string moveValue(int current, int target1, int target2) {
    // [->+>+<<]
    std::ostringstream oss;
    oss << "[-"
	<< movePtr(target1, current)
	<< increment(1)
	<< movePtr(target2, target1)
	<< increment(1)
	<< movePtr(current, target2)
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

  std::string movePtrUntilZero(int stride, bool unconditionalStart = false) {
    // [<] or [>]
    char const ch = (stride > 0) ? '>' : '<';
    std::string const move(std::abs(stride), ch);
    
    std::stringstream oss;
    if (unconditionalStart) oss << move;
    oss << "[" << move << "]";
    return oss.str();
  }

  std::string notValue(int current, int result, int copy, int tmp) {
    // Store !current in result
    // assumes copy and tmp are 0
    // leaves copy and tmp 0
    
    std::ostringstream oss;
    oss << movePtr(result, current)
	<< increment(1)
	<< movePtr(current, result)
	<< copyValue(current, copy, tmp)
	<< movePtr(copy, current)
	<< "["
	<<   zero()
	<<   movePtr(result, copy)
	<<   zero()
	<<   movePtr(copy, result)
	<< "]"
	<< movePtr(current, copy);

    return oss.str();
  }

  
  std::string movePtrUntilNonzero(int stride, int value, int flag, int tmp1, int tmp2) {
    // Assumes pointer points at value.
    // This algorithm will enter a loop by setting the flag and immidiately clearing it after entering
    // In the loop, the move (size stride) will be performed
    // Then, based on the value it landed on, it will decide if the loop needs to be re-entered.
    // If (value != 0) break from loop
    // This is implemented by first computing the NOT of the current value (see above).
    
    // 1. Set flag = 1 and enter loop
    // 2.   Clear flag
    // 3.   move
    // 4.   store not(value) in flag
    // 5.   if (flag zero) break, otherwise back to 2

    char const ch = (stride > 0) ? '>' : '<';
    std::string const move(std::abs(stride), ch);

    std::ostringstream oss;
    oss << movePtr(flag, value)
	<< increment(1)
	<< "["
	<<   zero()
	<<   move
	<<   movePtr(value, flag)
	<<   notValue(value, flag, tmp1, tmp2)
	<<   movePtr(flag, value)
	<< "]"
	<< movePtr(value, flag);
    

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

// MovePointerDynamic
TXT(MovePointerDynamic) { return "move_ptr_dyn"; }
GEN(MovePointerDynamic) {
  return Algorithm::movePtrUntilNonzero(stride * (dir == Left ? -1 : 1),
					value, flag, tmp1, tmp2);
}

// ZeroCell
TXT(ZeroCell) { return "zero_cell"; }
GEN(ZeroCell) { return Algorithm::zero(); }

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

// CopyData
TXT(CopyData) { return "copy_data"; } // TODO add indices
GEN(CopyData) {
  int const cur = current.resolve(ctx);
  int const dst = dest.resolve(ctx);
  int const tmp = scratch.resolve(ctx);
  return Algorithm::copyValue(cur, dst, tmp);
}

// Out
TXT(Out) { return "out"; }
GEN(Out) { return "."; }
