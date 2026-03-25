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

  class TypeSystem {
    
    std::unique_ptr<VoidType> _void;
    std::unique_ptr<IntegerType> _i8;
    std::unique_ptr<IntegerType> _i16;

    mutable std::vector<std::unique_ptr<RawType>> _rawTypes;    
    mutable std::vector<std::unique_ptr<ArrayType>> _arrayTypes;
    mutable std::vector<std::unique_ptr<StringType>> _stringTypes;
    mutable std::vector<std::unique_ptr<StructType>> _structTypes;

  public:
    // TODO: make return types more explicit: IntegerType const * instead of TypeHandle
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


    TypeHandle structT(std::string const &name) const {
      for (auto const &s: _structTypes) {
	if (s->str() == name) return s.get();
      }
      return nullptr; // no struct by this name exists
    }
    
    template <typename ... Args> 
    TypeHandle defineStruct(std::string const &name, Args&& ... args){
      if (structT(name) != nullptr) return nullptr; // already defined a struct with this name
      _structTypes.emplace_back(std::make_unique<StructType>(name, std::forward<Args>(args)...));
      return _structTypes.back().get();
    }
  };  
  
} // namespace types

namespace values {
  
  namespace impl {
    struct Base {
      types::TypeHandle _type;
      Base(types::TypeHandle t): _type(t) {}
      virtual ~Base() = default;

      virtual std::shared_ptr<Base> clone() const = 0;
      virtual std::string str() const = 0;            
      virtual types::TypeHandle type() const { return _type; }
      virtual int value() const { assert(false); return 0; }
      virtual std::string varName() const { assert(false); return ""; }
      virtual std::shared_ptr<Base> element(size_t) const { assert(false); return nullptr; }
      virtual std::shared_ptr<Base> field(std::string const &) const { assert(false); return nullptr; }
      virtual bool isRef() const { return false; }
    };

    struct Ref: Base {
      std::string _varName;
      Ref(Ref const &other): Base(nullptr), _varName(other._varName) {}
      Ref(std::string const &name): Base(nullptr), _varName(name) {}
      virtual std::string varName() const override { return _varName; }
      virtual std::string str() const override { return _varName; }
      virtual bool isRef() const override { return true; }
      virtual std::shared_ptr<Base> clone() const override { return std::make_shared<Ref>(*this); }      
    };

    struct i8: Base {
      int _value;
      i8(types::TypeHandle t, i8 const &other): Base(t), _value(other.value()) {}
      i8(types::TypeHandle t, int v): Base(t), _value(v & 0xff) {}
      i8(...): Base(nullptr) {}
      virtual int value() const override { return _value; }
      virtual std::string str() const override { return std::to_string(_value); }
      virtual std::shared_ptr<Base> clone() const override { return std::make_shared<i8>(*this); }      
    };

    struct i16: Base {
      int _value;
      i16(types::TypeHandle t, i16 const& other): Base(t), _value(other.value()) {}
      i16(types::TypeHandle t, int v): Base(t), _value(v & 0xffff) {}
      i16(...): Base(nullptr) {}
      
      virtual int value() const override { return _value; }
      virtual std::string str() const override { return std::to_string(_value); }
      virtual std::shared_ptr<Base> clone() const override { return std::make_shared<i16>(*this); }      
    };      

    struct string: Base {
      std::string const _str;
      std::vector<std::shared_ptr<Base>> arr;

      template <typename StringType> requires std::constructible_from<std::string, StringType>    
      string(types::TypeHandle t, types::TypeHandle ct, StringType const &s): Base(t), _str(s) {
	for (char c: _str)  arr.emplace_back(std::make_shared<i8>(ct, c));
	arr.emplace_back(std::make_shared<i8>(ct, 0));
      }
      
      string(string const &other):
	Base(other.type()),
	_str(other._str)
      {
	for (auto const &element: other.arr) {
	  arr.push_back(element->clone());
	}
      }

      string(...): Base(nullptr) {}
      
      virtual std::string str() const override { return std::string("\"") + _str + "\""; }
      virtual std::shared_ptr<Base> clone() const override { return std::make_shared<string>(*this); }
      virtual std::shared_ptr<Base> element(size_t idx) const override {
	assert(idx < _str.size() + 1);
	return arr[idx];
      }
    }; // string

    struct structT: Base {
      struct Field {
	std::string name;
	std::shared_ptr<Base> value;
      };

      std::vector<Field> _fields;
      
      template <typename ... Values> requires (std::convertible_to<std::remove_cvref_t<Values>, std::shared_ptr<Base>> && ...)
      structT(types::TypeHandle t, Values&& ... values):
	Base(t)
      {
	// Initialize the fields with only their names, taken from the typesystem.
	assert(this->type()->tag() == types::STRUCT);
	auto structType = static_cast<types::StructType const *>(this->type());
	for (auto const &f: structType->_fields) {
	  _fields.push_back({
	      .name = f.name,
	      .value = nullptr
	    });
	}
	
	// Now add the values from the parameter pack
	assert(_fields.size() == sizeof ... (Values));
	std::size_t i = 0;
	((_fields[i++].value = std::shared_ptr<Base>(std::forward<Values>(values))), ...);

	// Assert that all types match
	for (size_t i = 0; i != _fields.size(); ++i) {
	  assert(structType->_fields[i].type == _fields[i].value->type());
	}
      }

      structT(structT const &other):
	Base(other.type())
      {
	for (auto const &f: other._fields) {
	  _fields.push_back({
	      .name = f.name,
	      .value = f.value->clone()
	    });
	}
      }
      structT(...): Base(nullptr) {}
      
      virtual std::shared_ptr<Base> field(std::string const &name) const override {
	for (Field const &f: _fields) if (f.name == name) return f.value;
	assert(false && "invalid field name");
	std::unreachable();
      }

      virtual std::shared_ptr<Base> clone() const override { return std::make_shared<structT>(*this); }

      virtual std::string str() const override {
	std::ostringstream oss;
	oss << "{\n";
	for (auto const &f: _fields) {
	  oss << "  " << f.name << ": " << f.value->str() << '\n';
	}
	oss << "}";
	return oss.str();
      }
    }; // structT

    
    struct array: Base {
      types::TypeHandle elementType;
      std::vector<std::shared_ptr<Base>> arr;

      template <typename... Values>
      array(types::TypeHandle t, Values&&... values): Base(t), elementType(t->elementType()) {
	arr.reserve(sizeof...(Values));
	(arr.emplace_back(make_array_item(std::forward<Values>(values))), ...);

	// assert that all elements of of the correct type
	for (auto const &elem: arr) assert(elem->isRef() || elem->type() == elementType);
      }
      
      array(array const &other): Base(other.type()), elementType(other.elementType) {
	arr.reserve(other.arr.size());
	for (auto const& elem: other.arr) {
	  arr.emplace_back(make_array_item(elem));
	}
      }

      array(...): Base(nullptr) {}

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
	  return std::make_shared<Ref>(std::string(std::forward<Arg>(arg)));
	} else {
	  switch (elementType->tag()) {
	  case types::I8: return std::make_shared<i8>(elementType, std::forward<Arg>(arg));
	  case types::I16: return std::make_shared<i16>(elementType, std::forward<Arg>(arg));
	  case types::ARRAY: return std::make_shared<array>(elementType, std::forward<Arg>(arg));
	  case types::STRING: {
	    types::TypeHandle i8 = static_cast<types::StringType const *>(elementType)->elementType();
	    return std::make_shared<string>(elementType, i8, std::forward<Arg>(arg));
	  }
	  default: assert(false);
	  }
	}
      }
    }; // array

  }
  

  using Value = std::shared_ptr<impl::Base>;

  // Factories for each of the values
  inline Value i8(types::TypeSystem const &ts, int val) {
    return std::make_shared<impl::i8>(ts.i8(), val);
  }

  inline Value i16(types::TypeSystem const &ts, int val) {
    return std::make_shared<impl::i16>(ts.i16(), val);
  }

  inline Value string(types::TypeSystem const &ts, std::string const &str) {
    return std::make_shared<impl::string>(ts.string(str.length()), ts.i8(), str);
  }

  template <typename ... Values>
  inline Value structT(types::TypeSystem const &ts, std::string const &name, Values&& ... values) {
    return std::make_shared<impl::structT>(ts.structT(name), std::forward<Values>(values)...);
  }
  
  template <typename ... Elements>
  inline Value array(types::TypeSystem const &ts, types::TypeHandle elementType, Elements&& ... elems) {
    return std::make_shared<impl::array>(ts.array(elementType, sizeof ... (elems)),
					 std::forward<Elements>(elems)...);
  }

  using Ref = std::shared_ptr<impl::Ref>;

  inline Ref ref(std::string const &varName) {
    return std::make_shared<impl::Ref>(varName);
  }

  
} // namespace values

