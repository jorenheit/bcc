#include "bfc/types/type_rules.h"

#include "bfc/types/types.h"

namespace {

  using types::TypeHandle;
  using types::rules::OpResult;
  
  OpResult ok(TypeHandle t) { return {t, ""}; }
  OpResult fail(std::string msg) { return {types::null, std::move(msg)}; }

  bool sameType(TypeHandle a, TypeHandle b) { return a == b; }

  bool bothIntegers(TypeHandle a, TypeHandle b) {
    return types::isInteger(a) && types::isInteger(b);
  }

  bool pointerCanDecayInto(TypeHandle target, TypeHandle t) {
    assert(types::isPointer(t) && types::isPointer(target));

    if (t == target) return true;
    TypeHandle pointeeType = types::cast<types::PointerType>(t)->pointeeType();

    if (types::isArrayLike(pointeeType)) {
      auto arrayType = types::cast<types::ArrayLike>(pointeeType);
      return pointerCanDecayInto(target, TypeSystem::pointer(arrayType->elementType()));
    }
    if (types::isStruct(pointeeType)) {
      auto structType = types::cast<types::StructType>(pointeeType);
      return pointerCanDecayInto(target, TypeSystem::pointer(structType->fieldType(0)));
    }
    return false;
  }
  
  TypeHandle promotedInteger(TypeHandle a, TypeHandle b) {
    if (!bothIntegers(a, b)) return types::null;
    if (a->tag() == types::I16 || b->tag() == types::I16) {
      return (a->tag() == types::I16) ? a : b;
    }
    return a;
  }

  OpResult requireIntegers(BinOp op, TypeHandle lhs, TypeHandle rhs) {
    if (!types::isInteger(lhs) || !types::isInteger(rhs)) {
      return fail( "operator '" + binOpStr(op) + "' requires integer operands, got '" +
		  lhs->str() + "' and '" + rhs->str() + "'.");
    }
    return ok(promotedInteger(lhs, rhs));
  }

  bool isLogical(BinOp op) {
    switch(op) {
    case BinOp::And:
    case BinOp::Nand:
    case BinOp::Or:
    case BinOp::Nor:
    case BinOp::Xor:
    case BinOp::Xnor: return true;
    default: return false;
    }
    std::unreachable();
  }

  bool isComparison(BinOp op) {
    switch(op) {
    case BinOp::Eq:
    case BinOp::Neq:
    case BinOp::Lt:
    case BinOp::Le:
    case BinOp::Gt:
    case BinOp::Ge: return true;
    default: return false;
    }
    std::unreachable();
  }
  
} // namespace


OpResult types::rules::binOpResult(BinOp op, TypeHandle lhs, TypeHandle rhs) {
  // Add and Sub can interact with pointers
  if (op == BinOp::Add || op == BinOp::Sub) {
    if (bothIntegers(lhs, rhs)) return ok(promotedInteger(lhs, rhs));
    if (types::isPointer(lhs) && types::isInteger(rhs)) return ok(lhs);
    if (op == BinOp::Add && types::isInteger(lhs) && types::isPointer(rhs)) return ok(rhs);
    return fail("operator '" + binOpStr(op) + "' is not defined for '" +
		lhs->str() + "' and '" + rhs->str() +
		"'. Expected integer+integer or pointer+integer.");
  }

  if ((isLogical(op) || isComparison(op)) && bothIntegers(lhs, rhs)) {
    return ok(TypeSystem::i8());
  }
  
  // For all other operators, require both to be integers and return promoted type
  return requireIntegers(op, lhs, rhs);
}

OpResult types::rules::assignResult(TypeHandle dest, TypeHandle src) {
  if (dest == nullptr || src == nullptr) 
    return fail("cannot assign because one of the operand types is null.");
  if (sameType(dest, src))  return ok(dest);

  switch (dest->tag()) {
  case VOID: {
    return fail("cannot assign value of type '" + src->str() +
		"' to destination of type 'void'.");
  }
  case RAW: {
    if (src->size() <= dest->size()) return ok(dest);
    return fail("cannot assign value of type '" + src->str() +
		"' to destination of type '" + dest->str() +
		"': source size (" + std::to_string(src->size()) +
		") exceeds destination size (" + std::to_string(dest->size()) + ").");
  }
  case I8: {
    return fail("cannot assign value of type '" + src->str() +
		"' to destination of type 'i8'.");
  }
  case I16: {
    if (isInteger(src)) return ok(dest);
    return fail("cannot assign value of type '" + src->str() +
		"' to destination of type 'i16'.");
  }
  case ARRAY: {
    return fail("cannot assign value of type '" + src->str() +
		"' to destination of type '" + dest->str() +
		"': arrays are only assignable when their types match exactly.");
  }
  case STRING: {
    if (src->tag() != STRING)
      return fail("cannot assign value of type '" + src->str() +
		  "' to destination of type '" + dest->str() +
		  "': expected a string source.");
    if (src->size() > dest->size())
      return fail("cannot assign string of type '" + src->str() +
		  "' to destination of type '" + dest->str() +
		  "': source capacity exceeds destination capacity.");
    return ok(dest);
  }

  case STRUCT: {
    return fail("cannot assign value of type '" + src->str() +
		"' to destination of type '" + dest->str() +
		"': structs are only assignable when their types match exactly.");
  }
  case POINTER: {
    if (pointerCanDecayInto(dest, src) || isI16(src)) return ok(dest);
    return fail("cannot assign value of type '" + src->str() +
		"' to destination of type '" + dest->str() +
		"': expected an i16 or compatible pointer type.");
  }

  default: std::unreachable();
  }
}
