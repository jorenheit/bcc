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
    virtual Type const *fieldType(std::string const &) const { return nullptr; }
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
      return (this->tag() == I16) ? true : (other->tag() == I8);
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

    template <typename ... Args> requires (std::is_constructible_v<Field, Args> && ...)
    StructType(std::string const &name, Args&& ... args):
      _name(name),
      _size(0)
    {
      _fields.reserve(sizeof...(args));
      (_fields.emplace_back(std::forward<Args>(args)), ...);

      for (Field const &f: _fields) _size += f.type->size();
    }

    virtual TypeTag tag() const override { return STRUCT; }
    virtual int size() const override { return _size; }

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
    
    virtual std::string str() const { return _name; }
    virtual bool isConstructibleFrom(Type const *other) const override {
      return other == this;
    }    
  };
  
  inline bool isInteger(TypeHandle t) { return t->tag() == I8 || t->tag() == I16; }
  inline bool isArray(TypeHandle t)   { return t->tag() == ARRAY; }
  inline bool isString(TypeHandle t)   { return t->tag() == STRING; }
  inline bool isStruct(TypeHandle t)   { return t->tag() == STRUCT; }

  class TypeSystem {
    
    std::unique_ptr<VoidType> _void;
    std::unique_ptr<IntegerType> _i8;
    std::unique_ptr<IntegerType> _i16;

    mutable std::vector<std::unique_ptr<RawType>> _rawTypes;    
    mutable std::vector<std::unique_ptr<ArrayType>> _arrayTypes;
    mutable std::vector<std::unique_ptr<StringType>> _stringTypes;
    mutable std::vector<std::unique_ptr<StructType>> _structTypes;

  public:
    
    TypeSystem():
      _void(std::make_unique<VoidType>()),
      _i8(std::make_unique<IntegerType>(8)),
      _i16(std::make_unique<IntegerType>(16))
    {}

    TypeSystem(TypeSystem const &) = delete;
    TypeSystem &operator=(TypeSystem const &) = delete;
    
    inline TypeHandle voidT() const { return _void.get(); }
    inline TypeHandle i8() const    { return _i8.get(); }
    inline TypeHandle i16() const   { return _i16.get(); }

    inline TypeHandle array(TypeHandle elem, int length) const {
      for (auto const &ptr: _arrayTypes) {
	if (ptr->elementType() == elem && ptr->length() == length) return ptr.get();
      }
      _arrayTypes.emplace_back(std::make_unique<ArrayType>(elem, length));
      return _arrayTypes.back().get();
    }

    inline TypeHandle string(int maxLen) const {
      for (auto const &ptr: _stringTypes) {
	if (ptr->size() == maxLen + 1) {
	  TypeHandle t = ptr.get();
	  return t;
	}
      }
      _stringTypes.emplace_back(std::make_unique<StringType>(i8(), maxLen));
      return _stringTypes.back().get();
    }
    
    inline TypeHandle raw(int n) const {
      for (auto const &ptr: _rawTypes) {
	if (ptr->size() == n) return ptr.get();
      }
      _rawTypes.emplace_back(std::make_unique<RawType>(n));
      return _rawTypes.back().get();
    }


    template <typename ... Args> requires (std::is_constructible_v<StructType, std::string, Args> && ...)
    TypeHandle structT(std::string const &name, Args&& ... args){

      auto s1 = std::make_unique<StructType>(name, std::forward<Args>(args)...);

      for (auto const &s2: _structTypes) {
	if (s1->str() == s2->str()) {
	  // names match -> fields must match
	  if (s1->_fields.size() != s2->_fields.size()) return nullptr;
	  for (size_t i = 0; i != s1->_fields.size(); ++i) {
	    if ((s1->_fields[i].name != s2->_fields[i].name) ||
		(s1->_fields[i].type != s2->_fields[i].type)) return nullptr;
	  }
	  // found full match
	  return s2.get();
	}
      }

      // new definition -> add to known types
      _structTypes.emplace_back(std::move(s1));
      return _structTypes.back().get();
    }
  };  
  
} // namespace types

namespace values {
  
  namespace impl {
    struct Base {
      virtual ~Base() = default;
      virtual types::TypeHandle type(types::TypeSystem const &) const = 0;
      virtual std::shared_ptr<Base> clone() const = 0;
      virtual std::string str() const = 0;      
      virtual int value() const { assert(false); return 0; }
      virtual std::string varName() const { assert(false); return ""; }
      virtual std::shared_ptr<Base> element(size_t idx) const { assert(false); return nullptr; }
    };

    struct Var: Base {
      std::string _varName;
      Var(Base const &base): _varName(base.varName()) {}

      Var(std::string const &name): _varName(name) {}
      Var(...) { assert(false); }
      virtual types::TypeHandle type(types::TypeSystem const &ts) const override { return nullptr; }
      virtual std::string varName() const override { return _varName; }
      virtual std::string str() const override { return _varName; }
      
      std::shared_ptr<Base> clone() const override { return std::make_shared<Var>(*this); }      
    };

    struct i8: Base {
      int const _value;
      i8(i8 const &other): _value(other.value()) {}
      i8(int v) : _value(v & 0xff) {}
      i8(...): _value(0) { assert(false); }      
      virtual types::TypeHandle type(types::TypeSystem const &ts) const override { return ts.i8(); }
      virtual int value() const override { return _value; }
      virtual std::string str() const override { return std::to_string(_value); }
      virtual std::shared_ptr<Base> clone() const override { return std::make_shared<i8>(*this); }      
    };

    struct i16: Base {
      int const _value;
      i16(i16 const& other): _value(other.value()) {}
      i16(int v) : _value(v & 0xffff) {}
      i16(...): _value(0) { assert(false); }      
      virtual types::TypeHandle type(types::TypeSystem const &ts) const override { return ts.i16(); }
      virtual int value() const override { return _value; }
      virtual std::string str() const override { return std::to_string(_value); }
      std::shared_ptr<Base> clone() const override { return std::make_shared<i16>(*this); }      
    };      

    struct array: Base {
      types::TypeHandle elementType;
      std::vector<std::shared_ptr<Base>> arr;

      template <typename... Values>
      array(types::TypeHandle elementType, Values&&... values): elementType(elementType) {
	arr.reserve(sizeof...(Values));
	(arr.emplace_back(make_array_item(std::forward<Values>(values))), ...);
      }
      
      array(array const &other): elementType(other.elementType) {
	arr.reserve(other.arr.size());
	for (auto const& elem: other.arr) {
	  arr.emplace_back(make_array_item(elem));
	}
      }

      array(...) { assert(false); }      
      
      virtual types::TypeHandle type(types::TypeSystem const &ts) const override {
	return ts.array(elementType, arr.size());
      }

      virtual std::shared_ptr<Base> element(size_t idx) const override {
	assert(idx < arr.size());
	return arr[idx];
      }
    
      std::shared_ptr<Base> clone() const override { return std::make_shared<array>(*this); }      
      virtual std::string str() const override {
	std::ostringstream oss;
	oss << "{";
	for (size_t i = 0; i != arr.size(); ++i) {
	  oss << arr[i]->str();
	  if (i < arr.size() - 1) oss << ", ";
	}
	oss << "}";
	return oss.str();
      }

      
    private:
      template <typename Arg>
      std::shared_ptr<Base> make_array_item(Arg&& arg) {
	if constexpr (std::same_as<std::remove_cvref_t<Arg>, std::shared_ptr<Base>>) {
	  return arg->clone();
	} else if constexpr (std::constructible_from<std::string, Arg>) {
	  return std::make_shared<Var>(std::string(std::forward<Arg>(arg)));
	} else {
	  switch (elementType->tag()) {
	  case types::I8: return std::make_shared<i8>(std::forward<Arg>(arg));
	  case types::I16: return std::make_shared<i16>(std::forward<Arg>(arg));
	  case types::ARRAY: return std::make_shared<array>(std::forward<Arg>(arg));
	  default: assert(false);
	  }
	}
      }
    }; // array

    struct string: Base {
      std::string const _str;
      std::vector<std::shared_ptr<Base>> arr;

      template <typename StringType> requires std::constructible_from<std::string, StringType>    
      string(StringType const &s): _str(s) {
	for (char c: _str)  arr.emplace_back(std::make_shared<i8>(c));
	arr.emplace_back(std::make_shared<i8>(0));
      }
      
      string(string const &) = default;
      string(...): _str() { assert(false); }      

      virtual types::TypeHandle type(types::TypeSystem const &ts) const override { return ts.string(_str.size()); }
      virtual std::string str() const override { return std::string("\"") + _str + "\""; }
      std::shared_ptr<Base> clone() const override { return std::make_shared<string>(*this); }
      virtual std::shared_ptr<Base> element(size_t idx) const override {
	assert(idx < _str.size() + 1);
	return arr[idx];
      }
      
    }; // string
  }
  

  using Value = std::shared_ptr<impl::Base>;
  struct Var: std::shared_ptr<impl::Var> {
    Var() = default;
    
    template <typename NameType>  requires std::constructible_from<std::string, NameType>    
    Var(NameType const &name): std::shared_ptr<impl::Var>(std::make_shared<impl::Var>(std::string(name))) {}
  };
  using List = std::vector<Value>;
  

  template <typename ... Args> 
  Value value(types::TypeHandle type, Args&& ... args) {
    switch (type->tag()) {
    case types::I8:     return std::make_shared<impl::i8>(std::forward<Args>(args)...);
    case types::I16:    return std::make_shared<impl::i16>(std::forward<Args>(args)...);
    case types::ARRAY:  return std::make_shared<impl::array>(type->elementType(), std::forward<Args>(args)...);
    case types::STRING: return std::make_shared<impl::string>(std::forward<Args>(args)...);      
    default: assert(false);
    };
  }
  
  inline Var var(std::string const &varName) {
    return Var{varName};
  }

  
} // namespace values

