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
    VOID, RAW, I8, I16, ARRAY, STRUCT, POINTER
  };
  
  struct Type {
    ~Type() = default;
    virtual TypeTag tag() const = 0;
    virtual int size() const { return 1; }
    virtual int length() const { return 1; }
    virtual bool usesValue1() const { return false; }
    virtual Type const *elementType() const { return nullptr; }
  };

  using TypeHandle = Type const *;
  static constexpr TypeHandle null = nullptr;

  struct RawType: Type {
    int const _size;
    RawType(int n): _size(n) {}
    virtual TypeTag tag() const { return RAW; }
    virtual int size() const { return _size; }
  };
  
  struct VoidType: Type {
    virtual TypeTag tag() const override { return VOID; }
    virtual int size() const override { return 0; }
    virtual int length() const { return 0; }
  };

  struct IntegerType : Type {
    int const bits;
    IntegerType(int bits_): bits(bits_) {}
    virtual TypeTag tag() const override { return bits > 8 ? I16 : I8; }
    virtual bool usesValue1() const override { return bits > 8; }
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
  };

  inline bool isInteger(TypeHandle t) { return t->tag() == I8 || t->tag() == I16; }
  inline bool isArray(TypeHandle t)   { return t->tag() == ARRAY; }

  class TypeSystem {
    
    std::unique_ptr<VoidType> _void;
    std::unique_ptr<IntegerType> _i8;
    std::unique_ptr<IntegerType> _i16;

    mutable std::vector<std::unique_ptr<RawType>> _rawTypes;    
    mutable std::vector<std::unique_ptr<ArrayType>> _arrayTypes;
    
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
    
    inline TypeHandle raw(int n) const {
      for (auto const &ptr: _rawTypes) {
	if (ptr->size() == n) return ptr.get();
      }
      _rawTypes.emplace_back(std::make_unique<RawType>(n));
      return _rawTypes.back().get();
    }
  };  
  
} // namespace types

namespace values {
  
  namespace impl {
    struct Base {
      virtual ~Base() = default;
      virtual types::TypeHandle type(types::TypeSystem const &) const = 0;
      virtual std::shared_ptr<Base> clone() const = 0;
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
      virtual std::string varName() const { return _varName; }
      std::shared_ptr<Base> clone() const override { return std::make_shared<Var>(*this); }      
    };

    struct i8: Base {
      int const _value;
      i8(i8 const &other): _value(other.value()) {}
      i8(int v) : _value(v & 0xff) {}
      i8(...): _value(0) { assert(false); }      
      virtual types::TypeHandle type(types::TypeSystem const &ts) const override { return ts.i8(); }
      virtual int value() const override { return _value; }
      std::shared_ptr<Base> clone() const override { return std::make_shared<i8>(*this); }      
    };

    struct i16: Base {
      int const _value;
      i16(i16 const& other): _value(other.value()) {}
      i16(int v) : _value(v & 0xffff) {}
      i16(...): _value(0) { assert(false); }      
      virtual types::TypeHandle type(types::TypeSystem const &ts) const override { return ts.i16(); }
      virtual int value() const override { return _value; }
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
    case types::I8:    return std::make_shared<impl::i8>(std::forward<Args>(args)...);
    case types::I16:   return std::make_shared<impl::i16>(std::forward<Args>(args)...);
    case types::ARRAY: return std::make_shared<impl::array>(type->elementType(), std::forward<Args>(args)...);
    default: assert(false);
    };
  }
  
  inline Var var(std::string const &varName) {
    return Var{varName};
  }

  
} // namespace values

