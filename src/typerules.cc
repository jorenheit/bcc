#include "typerules.h"

namespace {

  using types::TypeHandle;
  using types::rules::OpResult;
  
  OpResult ok(TypeHandle t) { return {t, ""}; }
  OpResult fail(std::string msg) { return {types::null, std::move(msg)}; }

  bool sameType(TypeHandle a, TypeHandle b) { return a == b; }

  bool bothIntegers(TypeHandle a, TypeHandle b) {
    return types::isInteger(a) && types::isInteger(b);
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

  // For all other operators, require both to be integers
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
    if (sameType(dest, src) || isInteger(src)) return ok(dest);
    return fail("cannot assign value of type '" + src->str() +
		"' to destination of type '" + dest->str() +
		"': expected an integer or matching pointer type.");
  }

  default: std::unreachable();
  }
}
