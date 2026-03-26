#pragma once
#include <memory>
#include <vector>
#include <cassert>
#include <iostream>
#include <string>
#include <utility>
#include <type_traits>
#include <concepts>

namespace types {

  enum TypeTag {
    VOID, RAW, I8, I16, ARRAY, STRING, STRUCT, POINTER
  };
  
  struct Type {
    ~Type() = default;
    virtual TypeTag tag() const = 0;
    virtual int size() const { return 1; }
    virtual int length() const { return 1; }
    virtual bool usesValue1() const { return false; }
    virtual Type const *elementType() const { return nullptr; }
    virtual std::string fieldName(std::string const &) const { return ""; }
    virtual std::string fieldName(size_t) const { return ""; }    
    virtual Type const *fieldType(std::string const &) const { return nullptr; }
    virtual Type const *fieldType(size_t) const { return nullptr; }
    virtual std::string str() const = 0;
    virtual bool isConstructibleFrom(Type const *other) const = 0;
  };

  using TypeHandle = Type const *;
  static constexpr TypeHandle null = nullptr;

  struct RawType: Type {
    int const _size;
    RawType(int n): _size(n) {}
    virtual TypeTag tag() const { return RAW; }
    virtual int size() const { return _size; }
    virtual std::string str() const { return std::string("raw<") + std::to_string(_size) + ">"; }
    virtual bool isConstructibleFrom(Type const *other) const override {
      return other->size() <= _size;
    }
    
  };
  
  struct VoidType: Type {
    virtual TypeTag tag() const override { return VOID; }
    virtual int size() const override { return 0; }
    virtual int length() const { return 0; }
    virtual std::string str() const { return "void"; }
    virtual bool isConstructibleFrom(Type const *) const override { return false; }
  };

  struct IntegerType : Type {
    int const bits;
    IntegerType(int bits_): bits(bits_) {}
    virtual TypeTag tag() const override { return bits > 8 ? I16 : I8; }
    virtual bool usesValue1() const override { return bits > 8; }
    virtual std::string str() const { return tag() == I8 ? "i8" : "i16"; }
    virtual bool isConstructibleFrom(Type const *other) const override {
      return (this->tag() == I16) ? true : (this->tag() == I8 && other->tag() == I8);
    }
  };  

  struct ArrayType: Type {
    TypeHandle _elementType;
    int _length;

    ArrayType(TypeHandle elem, int len):  _elementType(elem), _length(len) {}
    virtual TypeTag tag() const override { return ARRAY; }
    virtual int size() const override { return _length * _elementType->size(); }
    virtual int length() const { return _length; }
    virtual bool usesValue1() const override { return _elementType->usesValue1(); }
    virtual TypeHandle elementType() const override { return _elementType; }
    virtual std::string str() const { return std::string("array<") + _elementType->str() + ", " + std::to_string(_length) + ">"; }
    virtual bool isConstructibleFrom(Type const *other) const override {
      if (size() < other->size()) return false;
      if (other->tag() == ARRAY)  return _elementType->isConstructibleFrom(other->elementType());
      if (other->tag() == STRING) return _elementType->tag() == I8 || _elementType->tag() == I16;
      return false;
    }
    
  };

  struct StringType: Type {
    TypeHandle _i8;
    int _capacity;

    StringType(TypeHandle i8, int maxLen):  _i8(i8), _capacity(maxLen + 1) {}
    virtual TypeTag tag() const override { return STRING; }
    virtual int size() const override { return _capacity; }
    virtual int length() const { return _capacity; }
    virtual TypeHandle elementType() const override { return _i8; }    
    virtual std::string str() const { return std::string("string<") + std::to_string(_capacity - 1) + ">"; }
    virtual bool isConstructibleFrom(Type const *other) const override {
      if (size() < other->size()) return false;
      if (other->tag() == ARRAY)  return other->elementType()->tag() == I8;
      if (other->tag() == STRING) return true;
      return false;
    }    
  };

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
    virtual int length() const override { return _fields.size(); }
    
    virtual bool usesValue1() const {
      for (Field const &f: _fields) if (f.type->usesValue1()) return true;
      return false;
    }
    
    virtual TypeHandle fieldType(std::string const &fieldName) const override {
      for (Field const &f: _fields) {
	if (f.name == fieldName) return f.type;
      }
      assert(false && "invalid field name");
      std::unreachable();
    }

    virtual Type const *fieldType(size_t index) const override {
      assert(index < _fields.size() && "index out of bounds");
      return _fields[index].type;
    }

    virtual std::string fieldName(std::string const &fieldName) const override {
      for (Field const &f: _fields) {
	if (f.name == fieldName) return f.name;
      }
      assert(false && "invalid field name");
      std::unreachable();
    }
    
    virtual std::string fieldName(size_t index) const override {
      assert(index < _fields.size() && "index out of bounds");
      return _fields[index].name;
    }    
    
    virtual std::string str() const { return _name; }
    virtual bool isConstructibleFrom(Type const *other) const override {
      return other == this;
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
  };
  
  inline bool isInteger(TypeHandle t) { return t->tag() == I8 || t->tag() == I16; }
  inline bool isArray(TypeHandle t)   { return t->tag() == ARRAY; }
  inline bool isString(TypeHandle t)   { return t->tag() == STRING; }
  inline bool isStruct(TypeHandle t)   { return t->tag() == STRUCT; }

} // namespace types
 
class TypeSystem {
    
  static std::unique_ptr<types::VoidType> _void;
  static std::unique_ptr<types::IntegerType> _i8;
  static std::unique_ptr<types::IntegerType> _i16;

  static std::vector<std::unique_ptr<types::RawType>> _rawTypes;    
  static std::vector<std::unique_ptr<types::ArrayType>> _arrayTypes;
  static std::vector<std::unique_ptr<types::StringType>> _stringTypes;
  static std::vector<std::unique_ptr<types::StructType>> _structTypes;

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
};  
