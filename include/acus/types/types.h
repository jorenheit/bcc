#pragma once
#include <memory>

#include "acus/core/data.h"

#define API_HEADER
#include "acus/api/api.h"

namespace acus::types {

  enum TypeTag {
    VOID, RAW, I8, I16, ARRAY, STRING, STRUCT, POINTER, FUNCTION, FUNCTION_POINTER
  };
  
  struct Type {
    virtual ~Type() = default;
    virtual TypeTag tag() const = 0;
    virtual std::string str() const = 0;
    virtual int size() const = 0;
    virtual bool usesValue1() const = 0;    
  };

  using TypeHandle = Type const *;
  static constexpr TypeHandle null = nullptr;

  struct NameTypePair {
    std::string name;
    types::TypeHandle type;
  };
  
  template <typename T> requires std::derived_from<T, Type>
  T const *cast(TypeHandle t) {
    T const *ptr = dynamic_cast<T const *>(t);
    assert(ptr != nullptr && "invalid type cast");
    return ptr;
  }

  struct RawType: Type {
    int const _size;
    RawType(int n): _size(n) {}
    virtual TypeTag tag() const { return RAW; }
    virtual int size() const { return _size; }
    virtual bool usesValue1() const { return true; }
    virtual std::string str() const { return std::string("raw<") + std::to_string(_size) + ">"; }
  }; // struct RawType
  
  struct VoidType: Type {
    virtual TypeTag tag() const override { return VOID; }
    virtual int size() const override { return 0; }
    virtual int length() const { return 0; }
    virtual bool usesValue1() const { return false; }    
    virtual std::string str() const { return "void"; }
  }; // struct VoidType

  struct IntegerType : Type {
    int const bits;
    IntegerType(int bits_): bits(bits_) {}
    virtual TypeTag tag() const override { return bits > 8 ? I16 : I8; }
    virtual int size() const override { return 1; }
    virtual bool usesValue1() const override { return bits > 8; }
    virtual std::string str() const override { return tag() == I8 ? "i8" : "i16"; }
  }; // struct IntegerType

  struct ArrayLike: Type {
    TypeHandle _elementType;
    int _length;

    ArrayLike(TypeHandle elem, int len): _elementType(elem), _length(len) {}
    int length() const { return _length; }
    TypeHandle elementType() const { return _elementType; }
    virtual int size() const override { return _length * _elementType->size(); }    
  }; // struct ArrayLike
  
  struct ArrayType: ArrayLike {
    ArrayType(TypeHandle elem, int len): ArrayLike(elem, len) {}
    virtual TypeTag tag() const override { return ARRAY; }
    virtual bool usesValue1() const override { return _elementType->usesValue1(); }
    virtual std::string str() const override {
      return std::string("array<") + _elementType->str() + ", " + std::to_string(_length) + ">";
    }    
  }; // struct ArrayType

  struct StringType: ArrayLike {
    StringType(TypeHandle charType, int maxLen):  ArrayLike(charType, maxLen + 1) {}
    virtual TypeTag tag() const override { return STRING; }
    virtual bool usesValue1() const override { return false; }
    virtual std::string str() const { return std::string("string<") + std::to_string(length() - 1) + ">"; }
  }; // struct StringType

  struct StructType: Type {
    std::string _name;
    std::vector<NameTypePair> _fields;
    int _size;

    StructType(std::string const &name, std::vector<NameTypePair> const &fields):
      _name(name),
      _fields(fields),
      _size(0)
    {
      for (auto const &f: _fields) _size += f.type->size();
    }

    virtual TypeTag tag() const override { return STRUCT; }
    virtual int size() const override { return _size; }
    
    virtual bool usesValue1() const {
      for (auto const &f: _fields) if (f.type->usesValue1()) return true;
      return false;
    }

    virtual std::string str() const { return _name; }

    int fieldCount() const { return _fields.size(); }

    int fieldIndex(std::string const &fieldName) const {
      for (size_t i = 0; i != _fields.size(); ++i) {
	if (_fields[i].name == fieldName) return static_cast<int>(i);
      }
      return -1;
    }
    
    TypeHandle fieldType(size_t index) const {
      assert(index < _fields.size() && "index out of bounds");
      return _fields[index].type;
    }
    
    TypeHandle fieldType(std::string const &fieldName) const {
      return fieldType(fieldIndex(fieldName));
    }

    int fieldOffset(size_t index) const {
      assert(index < _fields.size() && "index out of bounds");      
      int offset = 0;
      for (size_t i = 0; i != index; ++i) {
	offset += fieldType(i)->size();
      }
      return offset;
    }

    int fieldOffset(std::string const &fieldName) const {
      return fieldOffset(fieldIndex(fieldName));
    }
    
    std::string fieldName(size_t index) const {
      assert(index < _fields.size() && "index out of bounds");
      return _fields[index].name;
    }
  }; // struct StructType


  struct PointerType: Type {
    TypeHandle _pointeeType;
    PointerType(TypeHandle pointee): _pointeeType(pointee) {}
    
    virtual TypeTag tag() const override { return POINTER; }
    virtual int size() const override { return RuntimePointer::Size; }
    virtual bool usesValue1() const override { return true; }
    virtual std::string str() const override { return std::string("ptr<") + _pointeeType->str() + ">"; }

    TypeHandle pointeeType() const { return _pointeeType; }    
  }; // struct PointerType

  
  struct FunctionType: Type {
    TypeHandle _returnType;
    std::vector<TypeHandle> _paramTypes;

    template <typename ... ParamTypes>
    FunctionType(TypeHandle ret, ParamTypes ... params):
      _returnType(ret),
      _paramTypes({params ...})
    {}

    FunctionType(TypeHandle ret, std::vector<TypeHandle> const &paramTypes):
      _returnType(ret),
      _paramTypes(paramTypes)
    {}

    TypeHandle returnType() const { return _returnType; }
    std::vector<TypeHandle> const &paramTypes() const { return _paramTypes; }
    
    virtual TypeTag tag() const override { return FUNCTION; }
    virtual int size() const override { assert(false); std::unreachable(); }
    virtual bool usesValue1() const override { assert(false); std::unreachable(); }
    virtual std::string str() const override {
      std::string ret = _returnType->str() + "(";
      for (size_t i = 0; i != _paramTypes.size(); ++i) {
	ret += _paramTypes[i]->str();
	if (i < _paramTypes.size() - 1)
	  ret += ", ";
      }
      ret += ")";
      return ret;
    }
  };

  struct FunctionPointerType: Type {
    FunctionType const *_functionType;

    FunctionPointerType(FunctionType const *f): _functionType(f) {}
    
    virtual TypeTag tag() const override { return FUNCTION_POINTER; }
    virtual int size() const override { return 1; }
    virtual bool usesValue1() const override { return true; }
    virtual std::string str() const override { return std::string("fptr<") + _functionType->str() + ">"; }
    FunctionType const *functionType() const { return _functionType; }    
  };
  
  // Convenience functions to check type categories
  inline bool isI8(types::TypeHandle t)       { return t->tag() == types::I8; }
  inline bool isI16(types::TypeHandle t)      { return t->tag() == types::I16; }
  inline bool isInteger(TypeHandle t)         { return isI8(t) || isI16(t); }
  inline bool isArray(TypeHandle t)           { return t->tag() == ARRAY; }
  inline bool isString(TypeHandle t)          { return t->tag() == STRING; }
  inline bool isArrayLike(TypeHandle t)       { return isArray(t) || isString(t); }
  inline bool isStruct(TypeHandle t)          { return t->tag() == STRUCT; }
  inline bool isPointer(TypeHandle t)         { return t->tag() == POINTER; }
  inline bool isFunction(TypeHandle t)        { return t->tag() == FUNCTION; }
  inline bool isFunctionPointer(TypeHandle t) { return t->tag() == FUNCTION_POINTER; }

} // namespace acus::types
 
