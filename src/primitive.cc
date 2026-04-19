#include <sstream>
#include <iostream>
#include "primitive.h"
#include "data.h"
#include "util.h"


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

  // TODO:
  // namespace Pointer {
  //   static int pos;
  // };

  // void setPointer(int current) {
  //   Pointer::pos = current;
  // }

  // void assertCurrent(int current) {
  //   assert(current = Pointer::pos);
  // }
  
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

  // Move current into target. Leaves current at 0
  std::string moveValue(int current, int target) {
    assert(util::allDifferent(current, target));
    
    // [->+<]
    std::ostringstream oss;
    oss << movePtr(target, current)
	<< zero()
	<< movePtr(current, target) 
	<< "["
	<<   decrement()
	<<   movePtr(target, current)
	<<   increment()
	<<   movePtr(current, target)
	<< "]";
    
    return oss.str();
  }

  // Move current into both targets. Leaves current at 0
  std::string moveValue(int current, int target1, int target2) {
    assert(util::allDifferent(current, target1, target2));
    // [->+>+<<]
    std::ostringstream oss;
    oss << movePtr(target1, current)
	<< zero()
	<< movePtr(target2, target1)
	<< zero()
	<< movePtr(current, target2) 
	<< "["
	<<   decrement()
	<<   movePtr(target1, current)
	<<   increment()
	<<   movePtr(target2, target1)
	<<   increment()
	<<   movePtr(current, target2)
	<< "]";
    return oss.str();
  }

  // Copy current to target.
  std::string copyValue(int current, int target, int tmp) {
    assert(util::allDifferent(current, target, tmp));
    // [->+>+<<]>>[-<<+>>]<<
    std::ostringstream oss;
    oss << moveValue(current, target, tmp)
	<< movePtr(tmp, current)
	<< moveValue(tmp, current)
	<< movePtr(current, tmp);
    return oss.str();
  }

  // Set current (offset 0) to value
  std::string setToValue(int val) {
    std::ostringstream oss;
    oss << zero() << increment(val & 0xff);
    return oss.str();
  }

  // Set current (low-byte) and high-byte to value
  std::string setToValue(int val, int highByte) {
    std::ostringstream oss;
    oss << zero() << increment(val & 0xff)
	<< movePtr(highByte, 0)
	<< zero() << increment((val >> 8) & 0xff)
	<< movePtr(0, highByte);
    
    return oss.str();
  }

  // Store !current back into current  
  std::string notValue(int current, int tmp) {
    assert(util::allDifferent(current, tmp));

    std::ostringstream oss;
    oss << movePtr(tmp, current)
	<< setToValue(1)
	<< movePtr(current, tmp)
	<< "["
	<<   zero()
	<<   movePtr(tmp, current)
	<<   zero()
	<<   movePtr(current, tmp)
	<< "]"
	<< movePtr(tmp, current)
	<< moveValue(tmp, current)
	<< movePtr(current, tmp);
    
    return oss.str();
  }
  
  // Destructive OR (result in current, destroys other)
  std::string orValues(int current, int other, int tmp) {
    assert(util::allDifferent(current, other, tmp));
    
    std::ostringstream oss;
    oss << moveValue(current, tmp)
	<< movePtr(tmp, current)
	<< "["
	<<   zero()
	<<   movePtr(current, tmp)
	<<   setToValue(1)
	<<   movePtr(tmp, current)
	<< "]"
	<< movePtr(other, tmp)
	<< "["
	<<   zero()
	<<   movePtr(current, other)
	<<   setToValue(1)
	<<   movePtr(other, current)
	<< "]"
	<<  movePtr(current, other);

    return oss.str();
  }

  // Destructive AND (result in current, destroys other)
  std::string andValues(int current, int other, int tmp) {
    assert(util::allDifferent(current, other, tmp));
    
    std::ostringstream oss;
    oss << moveValue(current, tmp)
	<< movePtr(tmp, current)
	<< "["
	<<   zero()
	<<   movePtr(other, tmp)
	<<   "["
	<<     zero()
	<<     movePtr(current, other)
	<<     setToValue(1)
	<<     movePtr(other, current)
	<<   "]"
	<<   movePtr(tmp, other)
	<< "]"
	<<  movePtr(current, tmp);

    return oss.str();
  }

  // Destructive NAND (result in current, destroys other)
  std::string nandValues(int current, int other, int tmp) {
    assert(util::allDifferent(current, other, tmp));
    
    std::ostringstream oss;
    oss << andValues(current, other, tmp)
	<< notValue(current, tmp);

    return oss.str();
  }

  // Destructive NOR (result in current, destroys other)
  std::string norValues(int current, int other, int tmp) {
    assert(util::allDifferent(current, other, tmp));
    
    std::ostringstream oss;
    oss << orValues(current, other, tmp)
	<< notValue(current, tmp);

    return oss.str();
  }
  
  // Compare to constant value (destructive)
  std::string cmpConst(int value, int current, int tmp) {
    assert(util::allDifferent(current, tmp));
    
    std::ostringstream oss;
    oss << decrement(value)
	<< notValue(current, tmp);
    return oss.str();
  }

  // Add other to current, destroys other
  std::string add(int current, int other) {
    assert(util::allDifferent(current, other));

    std::ostringstream oss;
    oss << movePtr(other, current)
	<< "["
	<<   decrement()
	<<   movePtr(current, other)
	<<   increment()
	<<   movePtr(other, current)
	<< "]"
	<< movePtr(current, other);
      
    return oss.str();
  }

  // Subtract other from current, destroys other
  std::string subtract(int current, int other) {
    assert(util::allDifferent(current, other));

    std::ostringstream oss;
    oss << movePtr(other, current)
	<< "["
	<<   decrement()
	<<   movePtr(current, other)
	<<   decrement()
	<<   movePtr(other, current)
	<< "]"
	<< movePtr(current, other);
      
    return oss.str();
  }
  

  std::string boolean(int current, int tmp) {
    assert(util::allDifferent(current, tmp));

    std::ostringstream oss;
    oss << moveValue(current, tmp)
	<< movePtr(tmp, current)
	<< "["
	<<   zero()
	<<   movePtr(current, tmp)
	<<   setToValue(1)
	<<   movePtr(tmp, current)
	<< "]"
	<< movePtr(current, tmp);
    return oss.str();
  }

  // Decrement current and other until either (or both) becomes zero
  std::string reducePair(int current, int other, int tmp1, int tmp2) {
    assert(util::allDifferent(current, other, tmp1, tmp2));

    std::ostringstream oss;
    oss << copyValue(current, tmp1, tmp2)   // tmp1 = current
	<< movePtr(tmp1, current)
	<< "["
	<<   zero()                         // tmp1 = 0 unless we rebuild it below
	<<   movePtr(other, tmp1)
	<<   copyValue(other, tmp2, tmp1)   // tmp2 = other, tmp1 reused as scratch
	<<   movePtr(tmp2, other)
	<<   "["                            // execute once iff other != 0
	<<     zero()                       // make this a single-shot loop
	<<     movePtr(current, tmp2)
	<<     decrement()
	<<     movePtr(other, current)
	<<     decrement()
	<<     movePtr(current, other)
	<<     copyValue(current, tmp1, tmp2) // tmp1 = new current, tmp2 reused as scratch
	<<     movePtr(tmp2, current)
	<<   "]"
	<<   movePtr(tmp1, tmp2)
	<< "]"
	<< movePtr(current, tmp1);

    return oss.str();
  }
  
  // Compute current < other and stores the result in current. Other is destroyed
  // Result: 0 if false, any nonzero if true
  std::string less(int current, int other, int tmp1, int tmp2) {
    assert(util::allDifferent(current, other, tmp1, tmp2));

    // Reduce pair -> if current < other, other is nonzero
    std::ostringstream oss;
    oss << reducePair(current, other, tmp1, tmp2)
	<< movePtr(tmp1, current)
	<< setToValue(1)
	<< movePtr(other, tmp1)
	<< "["
	<<   zero()
        <<   movePtr(current, other)
	<<   setToValue(1)
	<<   movePtr(tmp1, current)
	<<   setToValue(0)
	<<   movePtr(other, tmp1)
	<< "]"
	<< movePtr(tmp1, other)
	<< "["
	<<   zero()
	<<   movePtr(current, tmp1)
	<<   setToValue(0)
	<<   movePtr(tmp1, current)
	<< "]"
	<< movePtr(current, tmp1);
      
    return oss.str();
  }

  // Compute current <= other and stores the result in current. Other is destroyed
  // Result: 0 if false, any nonzero if true
  std::string lessOrEqual(int current, int other, int tmp1, int tmp2) {
    assert(util::allDifferent(current, other, tmp1, tmp2));

    // Reduce pair -> if current <= other, current == 0 -> return not(current)
    std::ostringstream oss;
    oss << reducePair(current, other, tmp1, tmp2)
	<< movePtr(other, current)
	<< zero()
	<< movePtr(current, other)
	<< notValue(current, tmp1);
      
    return oss.str();
  }
  
  // Compute current > other and stores the result in current. Other is destroyed
  // Result: 0 if false, any nonzero if true  
  std::string greater(int current, int other, int tmp1, int tmp2) {
    assert(util::allDifferent(current, other, tmp1, tmp2));

    // Reduce pair -> if current > other, current is nonzero
    std::ostringstream oss;
    oss << reducePair(current, other, tmp1, tmp2)
	<< moveValue(current, other)
      	<< movePtr(tmp1, current)
	<< setToValue(1)
	<< movePtr(other, tmp1)
	<< "["
	<<   zero()
	<<   movePtr(current, other)
	<<   setToValue(1)
	<<   movePtr(tmp1, current)
	<<   setToValue(0)
	<<   movePtr(other, tmp1)
	<< "]"
	<< movePtr(tmp1, other)
	<< "["
	<<   zero()
	<<   movePtr(current, tmp1)
	<<   setToValue(0)
	<<   movePtr(tmp1, current)
	<< "]"
	<< movePtr(current, tmp1);
      
    return oss.str();
  }

  // Compute current >= other and stores the result in current. Other is destroyed
  // Result: 0 if false, any nonzero if true  
  std::string greaterOrEqual(int current, int other, int tmp1, int tmp2) {
    assert(util::allDifferent(current, other, tmp1, tmp2));

    // Reduce pair -> if current >= other, other == 0 -> return not(other)
    std::ostringstream oss;
    oss << reducePair(current, other, tmp1, tmp2)
	<< movePtr(other, current)
	<< notValue(other, tmp1)
	<< moveValue(other, current)
	<< movePtr(current, other);
      
    return oss.str();
  }
  
  // Compute current == other and stores the result in current. Other is destroyed
  // Result: 0 if false, any nonzero if true    
  std::string equal(int current, int other, int tmp1, int tmp2) {
    assert(util::allDifferent(current, other, tmp1, tmp2));

    // Reduce pair -> if current == other, both are zero -> return nor(current, other)
    std::ostringstream oss;
    oss << reducePair(current, other, tmp1, tmp2)
	<< movePtr(current, tmp1)
	<< norValues(current, other, tmp1);
      
    return oss.str();
  }
  
  
}


#define TXT(Name) std::string primitive::Name::text(Context const &ctx) const 
#define GEN(Name) std::string primitive::Name::generate(Context const &ctx) const

// Comment
TXT(Comment) { return txt; }
GEN(Comment) { return ""; }

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
  auto [cur, dst] = defer::resolve(ctx, current, dest);
  return Algorithm::moveValue(cur, dst);
}

// CopyData
TXT(CopyData) { return "copy_data"; } // TODO add indices
GEN(CopyData) {
  auto [cur, dst, tmp] = defer::resolve(ctx, current, dest, scratch);
  return Algorithm::copyValue(cur, dst, tmp);
}


// Not (destructive)
TXT(Not) { return "not"; } 
GEN(Not) {
  auto [cur, tmp] = defer::resolve(ctx, current, scratch);
  return Algorithm::notValue(cur, tmp);
}


// Or (destructive)
TXT(Or) { return "or"; } 
GEN(Or) {
  auto [cur, oth, tmp] = defer::resolve(ctx, current, other, scratch);
  return Algorithm::orValues(cur, oth, tmp);
}


// And (destructive)
TXT(And) { return "and"; } 
GEN(And) {
  auto [cur, oth, tmp] = defer::resolve(ctx, current, other, scratch);
  return Algorithm::andValues(cur, oth, tmp);
}


// Nor (destructive)
TXT(Nor) { return "nor"; } 
GEN(Nor) {
  auto [cur, oth, tmp] = defer::resolve(ctx, current, other, scratch);
  return Algorithm::norValues(cur, oth, tmp);
}


// Nand (destructive)
TXT(Nand) { return "nand"; } 
GEN(Nand) {
  auto [cur, oth, tmp] = defer::resolve(ctx, current, other, scratch);
  return Algorithm::nandValues(cur, oth, tmp);
}


// Cmp
TXT(Cmp) { return "cmp"; } 
GEN(Cmp) {
  auto [val, cur, tmp] = defer::resolve(ctx, value, current, scratch);
  return Algorithm::cmpConst(val, cur, tmp);
}

// Out
TXT(Out) { return "out"; }
GEN(Out) { return "."; }


// Add
TXT(Add) { return "add"; } 
GEN(Add) {
  auto [cur, oth] = defer::resolve(ctx, current, other);
  return Algorithm::add(cur, oth);
}

// Subtract
TXT(Subtract) { return "subtract"; } 
GEN(Subtract) {
  auto [cur, oth] = defer::resolve(ctx, current, other);
  return Algorithm::subtract(cur, oth);
}

// Less
TXT(Less) { return "less"; } 
GEN(Less) {
  auto [cur, oth, tmp1, tmp2] = defer::resolve(ctx, current, other, scratch1, scratch2);
  return Algorithm::less(cur, oth, tmp1, tmp2);
}

// LessOrEqual
TXT(LessOrEqual) { return "less_or_equal"; } 
GEN(LessOrEqual) {
  auto [cur, oth, tmp1, tmp2] = defer::resolve(ctx, current, other, scratch1, scratch2);
  return Algorithm::lessOrEqual(cur, oth, tmp1, tmp2);
}


// Greater
TXT(Greater) { return "greater"; } 
GEN(Greater) {
  auto [cur, oth, tmp1, tmp2] = defer::resolve(ctx, current, other, scratch1, scratch2);
  return Algorithm::greater(cur, oth, tmp1, tmp2);
}

// GreaterOrEqual
TXT(GreaterOrEqual) { return "greater_or_equal"; } 
GEN(GreaterOrEqual) {
  auto [cur, oth, tmp1, tmp2] = defer::resolve(ctx, current, other, scratch1, scratch2);
  return Algorithm::greaterOrEqual(cur, oth, tmp1, tmp2);
}


// Equal
TXT(Equal) { return "equal"; } 
GEN(Equal) {
  auto [cur, oth, tmp1, tmp2] = defer::resolve(ctx, current, other, scratch1, scratch2);
  return Algorithm::equal(cur, oth, tmp1, tmp2);
}
