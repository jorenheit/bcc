#pragma once
#include <memory>
#include <vector>
#include <cassert>
#include <string>
#include <utility>
#include <type_traits>
#include <concepts>
#include "types.h"

namespace values {
  
  namespace impl {
    struct Base {
      types::TypeHandle _type;
      Base(types::TypeHandle t): _type(t) {}
      virtual ~Base() = default;

      // Value specific members
      virtual types::TypeHandle type() const { return _type; }      
      virtual std::shared_ptr<Base> clone() const = 0;
      virtual std::string str() const = 0;            
      virtual int value() const { assert(false); return 0; }
      virtual std::string varName() const { assert(false); return ""; }
      virtual std::shared_ptr<Base> element(size_t) const { assert(false); return nullptr; }
      virtual std::shared_ptr<Base> pointee() const { assert(false); return nullptr; }
      virtual std::shared_ptr<Base> field(std::string const &) const { assert(false); return nullptr; }
      virtual std::shared_ptr<Base> field(size_t idx) const { assert(false); return nullptr; }      
      virtual bool isRef() const { return false; }
    };

    struct Ref: Base {
      std::string _varName;
      Ref(Ref const &other) = default;
      Ref(std::string const &var): Base(nullptr), _varName(std::string(var)) {}
      
      virtual std::string varName() const override { return _varName; }
      virtual std::string str() const override { return _varName; }
      virtual bool isRef() const override { return true; }
      virtual std::shared_ptr<Base> clone() const override { return std::make_shared<Ref>(*this); }      
    };

    struct i8: Base {
      int _value;
      i8(i8 const &other) = default;
      i8(int v): Base(TypeSystem::i8()), _value(v & 0xff) {}
      i8(...): Base(nullptr) { assert(false); }
      virtual int value() const override { return _value; }
      virtual std::string str() const override { return std::to_string(_value); }
      virtual std::shared_ptr<Base> clone() const override { return std::make_shared<i8>(*this); }      
    };

    struct i16: Base {
      int _value;
      i16(i16 const& other) = default;
      i16(int v): Base(TypeSystem::i16()), _value(v & 0xffff) {}
      i16(...): Base(nullptr) { assert(false); }
      
      virtual int value() const override { return _value; }
      virtual std::string str() const override { return std::to_string(_value); }
      virtual std::shared_ptr<Base> clone() const override { return std::make_shared<i16>(*this); }      
    };      

    struct string: Base {
      std::string const _str;
      std::vector<std::shared_ptr<Base>> arr;

      template <typename StringType> requires std::constructible_from<std::string, StringType>    
      string(StringType const &s): Base(TypeSystem::string(s.length())), _str(s) {
	for (char c: _str)  arr.emplace_back(std::make_shared<i8>(c));
	arr.emplace_back(std::make_shared<i8>(0));
      }
      
      string(string const &other):
	Base(other.type()),
	_str(other._str)
      {
	for (auto const &element: other.arr) {
	  arr.push_back(element->clone());
	}
      }

      string(...): Base(nullptr) { assert(false); }
      
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
      structT(std::string const &name, Values&& ... values):
	Base(TypeSystem::structT(name))
      {
	// Initialize the fields with only their names
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
      structT(...): Base(nullptr) { assert(false); }
      
      virtual std::shared_ptr<Base> field(std::string const &name) const override {
	for (Field const &f: _fields) if (f.name == name) return f.value;
	assert(false && "invalid field name");
	std::unreachable();
      }

      virtual std::shared_ptr<Base> field(size_t idx) const override {
	assert(idx < _fields.size() && "field index out of bounds");
	return _fields[idx].value;
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

    struct pointer: Base {
      std::shared_ptr<Ref> ref;

      pointer(types::TypeHandle pointee, std::string const &ref):
	Base(TypeSystem::pointer(pointee)),
	ref(std::make_shared<Ref>(ref))
      {}

      pointer(pointer const &) = default;
      pointer(...): Base(nullptr) { assert(false); }
      
      virtual std::shared_ptr<Base> clone() const override {
	return std::make_shared<pointer>(*this);
      }      
	    
      virtual std::string str() const override {
	return std::string("ptr<") + ref->str() + ">";
      }

      virtual std::shared_ptr<Base> pointee() const override { return ref; }
      
    };
    
    struct array: Base {
      types::TypeHandle elementType;
      std::vector<std::shared_ptr<Base>> arr;

      template <typename... Values>
      array(types::TypeHandle elementType, Values&&... values):
	Base(TypeSystem::array(elementType, sizeof...(Values))),
	elementType(elementType)
      {
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

      array(...): Base(nullptr) { assert(false); }

      virtual std::shared_ptr<Base> element(size_t idx) const override {
	assert(idx < arr.size());
	return arr[idx];
      }
    
      virtual std::shared_ptr<Base> clone() const override { return std::make_shared<array>(*this); }      

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
	  case types::I8: return std::make_shared<i8>(std::forward<Arg>(arg));
	  case types::I16: return std::make_shared<i16>(std::forward<Arg>(arg));
	  case types::ARRAY: return std::make_shared<array>(std::forward<Arg>(arg));
	  case types::STRING: return std::make_shared<string>(std::forward<Arg>(arg));
	  case types::POINTER: return std::make_shared<pointer>(std::forward<Arg>(arg));
	  default: assert(false);
	  }
	}
      }
    }; // array



  } // namespace impl
  
  using Value = std::shared_ptr<impl::Base>;

  // Factories for each of the values
  inline Value i8(int val) {
    return std::make_shared<impl::i8>(val);
  }

  inline Value i16(int val) {
    return std::make_shared<impl::i16>(val);
  }

  inline Value string(std::string const &str) {
    return std::make_shared<impl::string>(str);
  }

  template <typename ... Values>
  inline Value structT(std::string const &name, Values&& ... values) {
    return std::make_shared<impl::structT>(name, std::forward<Values>(values)...);
  }

  inline Value pointer(types::TypeHandle pointee, std::string const &var) {
    return std::make_shared<impl::pointer>(pointee, var);
  }
  
  template <typename ... Elements>
  inline Value array(types::TypeHandle elementType, Elements&& ... elems) {
    return std::make_shared<impl::array>(elementType, std::forward<Elements>(elems)...);
  }

  using Ref = std::shared_ptr<impl::Ref>;

  inline Ref ref(std::string const &varName) {
    return std::make_shared<impl::Ref>(varName);
  }

  
} // namespace values

