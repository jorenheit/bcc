#include <sstream>
#include <iostream>
#include "acus/ir/primitive.h"
#include "acus/core/data.h"
#include "acus/util/util.h"

using namespace acus;

std::string primitive::Sequence::dumpText(Context const &ctx) {
  std::ostringstream oss;
  for (auto const &node: nodes) {
    std::string str = node->text(ctx);
    if (not str.empty()) {
      oss << str << '\n';
    }
  }
  return oss.str();
}


std::string primitive::Sequence::dumpCode(Context const &ctx) {
  std::ostringstream oss;
  for (auto const &node: nodes) oss << node->generate(ctx);
  return oss.str();  
}


namespace acus::constants {
  using FactorPair = std::pair<uint8_t, uint8_t>;
  extern std::array<FactorPair, 129> table;
};


namespace acus::Algorithm {

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
    if (n == 0) return "";
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
  
  std::string setToValue(unsigned int val, int current, int tmp) {
    val &= 0xff;
    bool const countBack = val > 128;
    if (countBack) val = 256 - val;
    
    auto const naive = [&] -> std::string {
      std::ostringstream oss;
      oss << zero() << (countBack ? decrement(val) : increment(val));
      return oss.str();
    };
    
    auto const smart = [&] -> std::string {
      auto const [a, b] = constants::table[val];
      std::ostringstream oss;
      oss << zero()
	  << movePtr(tmp, current)
	//	  << zero()
	  << increment(a)
	  << "["
	  <<   decrement()
	  <<   movePtr(current, tmp)
	  <<   (countBack ? decrement(b) : increment(b))
	  <<   movePtr(tmp, current)
	  << "]"
	  << movePtr(current, tmp)
	  << modify((val - a * b) * (countBack ? -1 : 1));

      return oss.str();
    };

    //    return naive();
    
    std::string const smartResult = smart();
    std::string const naiveResult = naive();
    return smartResult.length() < naiveResult.length()
      ? smartResult
      : naiveResult;
  }
  
  
  // Store !!current back intor current
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

  // Destructive XOR (result in current, destroys other)
  std::string xorValues(int current, int other, int tmp1, int tmp2) {
    assert(util::allDifferent(current, other, tmp1, tmp2));

    int const result = tmp1;
    int const tmp = tmp2;
    
    std::ostringstream oss;
    oss << "["
	<<   movePtr(other, current)
	<<   notValue(other, tmp)
	<<   moveValue(other, result)
	<<   movePtr(current, other)
	<<   zero()
	<< "]"
	<< movePtr(other, current)
	<< "["
	<<   moveValue(other, result)
	<< "]"
	<< movePtr(result, other)  
	<< moveValue(result, current)
	<< movePtr(current, result);
    
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
  
  // Equal
  std::string equal(int current, int other) {
    assert(util::allDifferent(current, other));
    
    std::ostringstream oss;
    oss << subtract(current, other)
	<< notValue(current, other); // use empty other as tmp
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
  
  
}


#define TXT(Name) std::string primitive::Name::text(Context const &ctx) const 
#define GEN(Name) std::string primitive::Name::generate(Context const &ctx) const
#define MERGE(Name) std::shared_ptr<primitive::Node> primitive::Name::merge(Node const *other) const

// Comment
TXT(Comment) { return txt; }
GEN(Comment) { return ""; } // TODO: implement check for BF characters and just paste verbatim

// LoopOpen
TXT(LoopOpen) { return "LOOP_START: " + tag; }
GEN(LoopOpen) { return "["; }

// LoopClose
TXT(LoopClose) { return "LOOP_END: " + tag; }
GEN(LoopClose) { return "]"; }

// MovePointerRelative
TXT(MovePointerRelative) {
  int const n = amount.resolve(ctx);
  if (n == 0) return "";
  return ((n < 0) ? "LEFT: " : "RIGHT: ") + std::to_string(std::abs(n));
}

GEN(MovePointerRelative) {
  return Algorithm::movePtr(amount.resolve(ctx));
}

MERGE(MovePointerRelative) {
  if (auto ptr = dynamic_cast<MovePointerRelative const *>(other)) {
    return std::make_shared<MovePointerRelative>(amount + ptr->amount);
  }
  return nullptr;
}

// ZeroCell
TXT(ZeroCell) { return "ZERO"; }
GEN(ZeroCell) { return Algorithm::zero(); }
MERGE(ZeroCell) {
  if (dynamic_cast<ZeroCell const *>(other))     return std::make_shared<ZeroCell>();
  if (dynamic_cast<ZeroCellPlus const *>(other)) return std::make_shared<ZeroCell>();
  return nullptr;
}

// ZeroCellPlus
TXT(ZeroCellPlus) { return "ZERO+"; }
GEN(ZeroCellPlus) { return Algorithm::zeroPlus(); }
MERGE(ZeroCellPlus) {
  if (dynamic_cast<ZeroCellPlus const *>(other)) return std::make_shared<ZeroCellPlus>();
  if (dynamic_cast<ZeroCell const *>(other))     return std::make_shared<ZeroCellPlus>();
  return nullptr;
}

// ConstructConstant
TXT(ConstructConstant) { return "CONSTANT: " + std::to_string(value.resolve(ctx)); }
GEN(ConstructConstant) {
  auto const [val, cur, tmp] = defer::resolve(ctx, value, current, scratch);
  return Algorithm::setToValue(val, cur, tmp);
}


// ChangeBy
TXT(ChangeBy) {
  int const n = delta.resolve(ctx);
  if (n == 0) return "";
  return ((n > 0) ? "INC: " : "DEC: ") + std::to_string(std::abs(n));
}


GEN(ChangeBy) { return Algorithm::modify(delta.resolve(ctx)); }
MERGE(ChangeBy) {
  if (auto ptr = dynamic_cast<ChangeBy const *>(other))     return std::make_shared<ChangeBy>(delta + ptr->delta);
  if (dynamic_cast<ZeroCell const *>(other))     return std::make_shared<ZeroCell>();
  if (dynamic_cast<ZeroCellPlus const *>(other)) return std::make_shared<ZeroCellPlus>();
  return nullptr;
}

// MoveData
TXT(MoveData) {
  auto const [cur, dst] = defer::resolve(ctx, current, dest);
  if (cur == dst) return "";
  return "MOVE: " + std::to_string(dst - cur);
} 

GEN(MoveData) {
  auto const [cur, dst] = defer::resolve(ctx, current, dest);
  return Algorithm::moveValue(cur, dst);
}

MERGE(MoveData) {
  if (dynamic_cast<ZeroCell const *>(other))     return std::make_shared<MoveData>(*this);
  if (dynamic_cast<ZeroCellPlus const *>(other)) return std::make_shared<MoveData>(*this);
  return nullptr;
}

// CopyData
TXT(CopyData) {
  auto const [cur, dst] = defer::resolve(ctx, current, dest);
  if (cur == dst) return "";
  return "COPY: " + std::to_string(dst - cur);
}

GEN(CopyData) {
  auto [cur, dst, tmp] = defer::resolve(ctx, current, dest, scratch);
  return Algorithm::copyValue(cur, dst, tmp);
}

MERGE(CopyData) {
  if (dynamic_cast<ZeroCell const *>(other))     return std::make_shared<MoveData>(current, dest);
  if (dynamic_cast<ZeroCellPlus const *>(other)) return std::make_shared<MoveData>(current, dest);
  return nullptr;
}

// Compare to constant
TXT(Cmp) {
  int const val = value.resolve(ctx);
  return "CMP: " + std::to_string(val);
} 
GEN(Cmp) {
  auto [val, cur, tmp] = defer::resolve(ctx, value, current, scratch);
  return Algorithm::cmpConst(val, cur, tmp);
}

// Boolean (destructive)
TXT(Boolean) { return "BOOL"; } 
GEN(Boolean) {
  auto [cur, tmp] = defer::resolve(ctx, current, scratch);
  return Algorithm::boolean(cur, tmp);
}

// Not (destructive)
TXT(Not) { return "NOT"; } 
GEN(Not) {
  auto [cur, tmp] = defer::resolve(ctx, current, scratch);
  return Algorithm::notValue(cur, tmp);
}


// Or (destructive)
TXT(Or) {
  auto [cur, oth] = defer::resolve(ctx, current, other);
  return "OR: " + std::to_string(oth - cur);
} 
GEN(Or) {
  auto [cur, oth, tmp] = defer::resolve(ctx, current, other, scratch);
  return Algorithm::orValues(cur, oth, tmp);
}


// And (destructive)
TXT(And) {
  auto [cur, oth] = defer::resolve(ctx, current, other);
  return "AND: " + std::to_string(oth - cur);
}
GEN(And) {
  auto [cur, oth, tmp] = defer::resolve(ctx, current, other, scratch);
  return Algorithm::andValues(cur, oth, tmp);
}

// Xor (destructive)
TXT(Xor) {
  auto [cur, oth] = defer::resolve(ctx, current, other);
  return "XOR: " + std::to_string(oth - cur);
}
GEN(Xor) {
  auto [cur, oth, tmp1, tmp2] = defer::resolve(ctx, current, other, scratch1, scratch2);
  return Algorithm::xorValues(cur, oth, tmp1, tmp2);
}


// Out
TXT(Out) { return "OUT"; }
GEN(Out) { return "."; }


// Add
TXT(Add) { return "ADD"; } 
GEN(Add) {
  auto [cur, oth] = defer::resolve(ctx, current, other);
  return Algorithm::add(cur, oth);
}

// Subtract
TXT(Subtract) { return "SUB"; } 
GEN(Subtract) {
  auto [cur, oth] = defer::resolve(ctx, current, other);
  return Algorithm::subtract(cur, oth);
}

// Less
TXT(Less) { return "LT"; } 
GEN(Less) {
  auto [cur, oth, tmp1, tmp2] = defer::resolve(ctx, current, other, scratch1, scratch2);
  return Algorithm::less(cur, oth, tmp1, tmp2);
}

// LessOrEqual
TXT(LessOrEqual) { return "LE"; } 
GEN(LessOrEqual) {
  auto [cur, oth, tmp1, tmp2] = defer::resolve(ctx, current, other, scratch1, scratch2);
  return Algorithm::lessOrEqual(cur, oth, tmp1, tmp2);
}


// Greater
TXT(Greater) { return "GT"; } 
GEN(Greater) {
  auto [cur, oth, tmp1, tmp2] = defer::resolve(ctx, current, other, scratch1, scratch2);
  return Algorithm::greater(cur, oth, tmp1, tmp2);
}

// GreaterOrEqual
TXT(GreaterOrEqual) { return "GE"; } 
GEN(GreaterOrEqual) {
  auto [cur, oth, tmp1, tmp2] = defer::resolve(ctx, current, other, scratch1, scratch2);
  return Algorithm::greaterOrEqual(cur, oth, tmp1, tmp2);
}

// Equal
TXT(Equal) { return "EQ"; } 
GEN(Equal) {
  auto [cur, oth] = defer::resolve(ctx, current, other);
  return Algorithm::equal(cur, oth);
}
