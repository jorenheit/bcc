#pragma once
#include <memory>
#include <vector>
#include <cassert>
#include <iostream>
#include <string>
#include <utility>
#include <type_traits>
#include <concepts>
#include "data.h"

namespace types {

  enum TypeTag {
    VOID, RAW, I8, I16, ARRAY, STRING, STRUCT, POINTER
  };
  
  struct Type {
    ~Type() = default;
    virtual TypeTag tag() const = 0;
    virtual std::string str() const = 0;
    virtual bool isConstructibleFrom(Type const *other) const = 0;
    virtual int size() const = 0;
    virtual bool usesValue1() const = 0;    
  };

  using TypeHandle = Type const *;
  static constexpr TypeHandle null = nullptr; // TODO: am i using this?


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
    virtual bool isConstructibleFrom(Type const *other) const override {
      return other->size() <= _size;
    }
    
  }; // struct RawType
  
  struct VoidType: Type {
    virtual TypeTag tag() const override { return VOID; }
    virtual int size() const override { return 0; }
    virtual int length() const { return 0; }
    virtual bool usesValue1() const { return false; }    
    virtual std::string str() const { return "void"; }
    virtual bool isConstructibleFrom(Type const *) const override { return false; }
  }; // struct VoidType

  struct IntegerType : Type {
    int const bits;
    IntegerType(int bits_): bits(bits_) {}
    virtual TypeTag tag() const override { return bits > 8 ? I16 : I8; }
    virtual int size() const override { return 1; }
    virtual bool usesValue1() const override { return bits > 8; }
    virtual std::string str() const override { return tag() == I8 ? "i8" : "i16"; }
    virtual bool isConstructibleFrom(Type const *other) const override {
      return (this->tag() == I16) ? true : (this->tag() == I8 && other->tag() == I8);
    }
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
    virtual bool isConstructibleFrom(TypeHandle other) const override  {
      if (size() < other->size()) return false;
      if (other->tag() == ARRAY)  {
	return _elementType->isConstructibleFrom(cast<ArrayType>(other)->elementType());
      }
      if (other->tag() == STRING) return _elementType->tag() == I8 || _elementType->tag() == I16;
      return false;
    }
  }; // struct ArrayType

  struct StringType: ArrayLike {
    StringType(TypeHandle charType, int maxLen):  ArrayLike(charType, maxLen + 1) {} //_i8(i8), _capacity(maxLen + 1) {}
    virtual TypeTag tag() const override { return STRING; }
    virtual bool usesValue1() const override { return false; }
    virtual std::string str() const { return std::string("string<") + std::to_string(length() - 1) + ">"; }
    virtual bool isConstructibleFrom(Type const *other) const override {
      if (size() < other->size()) return false;
      if (other->tag() == ARRAY)  return cast<ArrayType>(other)->elementType()->tag() == I8;
      if (other->tag() == STRING) return true;
      return false;
    }
  }; // struct StringType

  struct StructType: Type {
    struct Field {
      std::string name;
      Type const *type;
    };

    std::string _name;
    std::vector<Field> _fields;
    int _size;

    template <typename ... Args>
    StructType(std::string const &name, Args&& ... args): _name(name), _size(0) {
      _fields.reserve(sizeof...(args));
      addFields(std::forward<Args>(args)...);
      for (Field const &f: _fields) _size += f.type->size();
    }

    virtual TypeTag tag() const override { return STRUCT; }
    virtual int size() const override { return _size; }
    
    virtual bool usesValue1() const {
      for (Field const &f: _fields) if (f.type->usesValue1()) return true;
      return false;
    }

    virtual std::string str() const { return _name; }
    virtual bool isConstructibleFrom(Type const *other) const override {
      return other == this;
    }

    int fieldCount() const { return _fields.size(); }

    int fieldIndex(std::string const &fieldName) const {
      for (size_t i = 0; i != _fields.size(); ++i) {
	if (_fields[i].name == fieldName) return static_cast<int>(i);
      }
      assert(false && "invalid field name");
      std::unreachable();
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

  private:
    void addFields() {}

    template <typename Name, typename... Rest>
    void addFields(Name&& name, Type const* type, Rest&&... rest) {
      _fields.push_back(Field{
	  std::string(std::forward<Name>(name)),
	  type
	});
      addFields(std::forward<Rest>(rest)...);
    }    
  }; // struct StructType


  struct PointerType: Type {
    Type const *_pointeeType;
    PointerType(Type const *pointee): _pointeeType(pointee) {}
    
    virtual TypeTag tag() const override { return POINTER; }
    virtual int size() const override { return RuntimePointer::Size; }
    virtual bool usesValue1() const override { return true; }
    virtual std::string str() const override { return std::string("ptr<") + _pointeeType->str() + ">"; }
    virtual bool isConstructibleFrom(Type const *other) const override {
      if (other->tag() == POINTER) return static_cast<PointerType const *>(other)->_pointeeType == this->_pointeeType;
      return (other->tag() == I8 || other->tag() == I16);
    }

    TypeHandle pointeeType() const { return _pointeeType; }    
  }; // struct PointerType


  // Convenience functions to check type categories
  inline bool isInteger(TypeHandle t) { return t->tag() == I8 || t->tag() == I16; }
  inline bool isArray(TypeHandle t)   { return t->tag() == ARRAY; }
  inline bool isString(TypeHandle t)   { return t->tag() == STRING; }
  inline bool isStruct(TypeHandle t)   { return t->tag() == STRUCT; }
  inline bool isPointer(TypeHandle t)   { return t->tag() == POINTER; }



  
} // namespace types
 
class TypeSystem {
    
  static std::unique_ptr<types::VoidType> _void;
  static std::unique_ptr<types::IntegerType> _i8;
  static std::unique_ptr<types::IntegerType> _i16;

  static std::vector<std::unique_ptr<types::RawType>> _rawTypes;    
  static std::vector<std::unique_ptr<types::ArrayType>> _arrayTypes;
  static std::vector<std::unique_ptr<types::StringType>> _stringTypes;
  static std::vector<std::unique_ptr<types::StructType>> _structTypes;
  static std::vector<std::unique_ptr<types::PointerType>> _pointerTypes;

public:
  static void init();
    
  static types::TypeHandle voidT() { return _void.get(); }
  static types::TypeHandle i8()    { return _i8.get(); }
  static types::TypeHandle i16()   { return _i16.get(); }

  static types::TypeHandle array(types::TypeHandle elem, int length) {
    for (auto const &ptr: _arrayTypes) {
      if (ptr->elementType() == elem && ptr->length() == length) return ptr.get();
    }
    _arrayTypes.emplace_back(std::make_unique<types::ArrayType>(elem, length));
    return _arrayTypes.back().get();
  }

  static types::TypeHandle string(int maxLen) {
    for (auto const &ptr: _stringTypes) {
      if (ptr->size() == maxLen + 1) {
	types::TypeHandle t = ptr.get();
	return t;
      }
    }
    _stringTypes.emplace_back(std::make_unique<types::StringType>(i8(), maxLen));
    return _stringTypes.back().get();
  }
    
  static types::TypeHandle raw(int n) {
    for (auto const &ptr: _rawTypes) {
      if (ptr->size() == n) return ptr.get();
    }
    _rawTypes.emplace_back(std::make_unique<types::RawType>(n));
    return _rawTypes.back().get();
  }


  static types::TypeHandle structT(std::string const &name) {
    for (auto const &s: _structTypes) {
      if (s->str() == name) return s.get();
    }
    return nullptr; // no struct by this name exists
  }
    
  template <typename ... Args> 
  static types::TypeHandle defineStruct(std::string const &name, Args&& ... args){
    if (structT(name) != nullptr) return nullptr; // already defined a struct with this name
    _structTypes.emplace_back(std::make_unique<types::StructType>(name, std::forward<Args>(args)...));
    return _structTypes.back().get();
  }

  static types::TypeHandle pointer(types::TypeHandle pointee) {
    for (auto const &ptr: _pointerTypes) {
      if (ptr->pointeeType() == pointee) {
	return ptr.get();
      }
    }
    _pointerTypes.emplace_back(std::make_unique<types::PointerType>(pointee));
    return _pointerTypes.back().get();
  }
};  
