#pragma once
#include <memory>
#include <vector>
#include <cassert>
#include <string>
#include <utility>
#include <type_traits>
#include <concepts>
#include <sstream>
#include "types.h"
#include "error.h"

namespace values {
  
  namespace impl {
    struct Base {
      types::TypeHandle _type;
      Base(types::TypeHandle t): _type(t) {}
      virtual ~Base() = default;

      virtual types::TypeHandle type() const { return _type; }      
      virtual std::shared_ptr<Base> clone() const = 0;
      virtual std::string str() const = 0;
      virtual bool isRef() const { return false; }
      virtual std::string varName() const { assert(false); std::unreachable(); }
    };

    struct Integer: Base {
      int _value;
      Integer(types::TypeHandle t, int v): Base(t), _value(v) {}

      virtual std::string str() const override { return std::to_string(_value); }
      int value() const { return _value; }      
    };
    
    struct i8: Integer {
      i8(i8 const &other) = default;
      i8(int v): Integer(TypeSystem::i8(), v & 0xff) {}
      virtual std::shared_ptr<Base> clone() const override { return std::make_shared<i8>(*this); }
    };

    struct i16: Integer {
      i16(i16 const& other) = default;
      i16(int v): Integer(TypeSystem::i16(), v & 0xffff) {}
      virtual std::shared_ptr<Base> clone() const override { return std::make_shared<i16>(*this); }      
    };      

    struct ArrayLike: Base {
      std::vector<std::shared_ptr<Base>> arr;

      ArrayLike(types::TypeHandle type):
	Base(type)
      {}

      std::shared_ptr<Base> element(size_t idx) const {
	assert(idx < arr.size());
	return arr[idx];
      }	
    };
    
    struct string: ArrayLike {
      std::string const _str;

      template <typename StringType> requires std::constructible_from<std::string, StringType>    
      string(StringType const &s):
	ArrayLike(TypeSystem::string(std::string(s).length())),
	_str(s)
      {
	for (char c: _str)  (ArrayLike::arr).emplace_back(std::make_shared<i8>(c));
	(ArrayLike::arr).emplace_back(std::make_shared<i8>(0));
      }
      
      string(string const &other):
	ArrayLike(other.type()),
	_str(other._str)
      {
	for (auto const &element: other.arr) {
	  arr.push_back(element->clone());
	}
      }

      virtual std::string str() const override {
	return std::string("\"") + _str + "\"";
      }
      
      virtual std::shared_ptr<Base> clone() const override {
	return std::make_shared<string>(*this);
      }

      std::shared_ptr<Base> element(size_t idx) const {
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
      structT(types::TypeHandle type, Values&& ... values):
	Base(type)
      {
	// Initialize the fields with only their names
	auto structType = types::cast<types::StructType>(type);
	for (int i = 0; i != structType->fieldCount(); ++i) {
	  _fields.push_back({
	      .name = structType->fieldName(i),
	      .value = nullptr
	    });
	}
	
	// Now add the values from the parameter pack
	assert(_fields.size() == sizeof ... (Values));
	std::size_t i = 0;
	((_fields[i++].value = std::shared_ptr<Base>(std::forward<Values>(values))), ...);

	// Assert that all types match
	for (size_t i = 0; i != _fields.size(); ++i) {
	  types::TypeHandle fieldType = _fields[i].value->type();
	  types::TypeHandle expectedType = structType->fieldType(i);
	  error::throw_if(fieldType != expectedType,
			  "unknown file", 0, 0, // TODO: can I fix this?
			  "expected type '" + expectedType->str() + "', got '" + fieldType->str() + "' in literal struct constructor.");
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
      
      std::shared_ptr<Base> field(std::string const &name) const {
	for (Field const &f: _fields) if (f.name == name) return f.value;
	assert(false && "invalid field name");
	std::unreachable();
      }

      std::shared_ptr<Base> field(size_t idx) const {
	assert(idx < _fields.size() && "field index out of bounds");
	return _fields[idx].value;
      }      
      
      virtual std::shared_ptr<Base> clone() const override {
	return std::make_shared<structT>(*this);
      }

      virtual std::string str() const override {
	std::ostringstream oss;
	oss << "{";
	for (auto const &f: _fields) {
	  oss << f.name << ": " << f.value->str() << ",";
	}
	oss << "}";
	return oss.str();
      }
    }; // structT

    struct array: ArrayLike {
      types::TypeHandle elementType;

      template <typename... Elements>
      explicit array(types::TypeHandle elementType, Elements&&... elements):
	ArrayLike(TypeSystem::array(elementType, sizeof...(Elements))),
	elementType(elementType)
      {
	static_assert((std::is_convertible_v<std::remove_cvref_t<Elements>, std::shared_ptr<Base>> && ...),
		      "values::array expects all elements to be Literal values");

	ArrayLike::arr.reserve(sizeof...(Elements));

	auto push = [&](std::shared_ptr<Base> const &value) {
	  error::throw_if(value->type() != elementType,
			  "unknown file", 0, 0, // TODO: can I fix this?
			  "expected type '" + elementType->str() + "', got '" + value->type()->str() + "' in literal array constructor.");
	  ArrayLike::arr.push_back(value);
	};
	
	(push(std::forward<Elements>(elements)), ...);
      }
      
      array(array const &other): ArrayLike(other.type()), elementType(other.elementType) {
	ArrayLike::arr.reserve(other.arr.size());
	for (auto const& elem: other.arr) {
	  ArrayLike::arr.emplace_back(elem->clone());
	}
      }

      virtual std::shared_ptr<Base> clone() const override {
	return std::make_shared<array>(*this);
      }      

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
    }; // array

    template <typename V> requires std::derived_from<V, Base>
    auto cast(std::shared_ptr<impl::Base> const &v) {
      auto ptr = std::dynamic_pointer_cast<V>(v);
      assert(ptr != nullptr && "invalid value cast");
      return ptr;
    }

  } // namespace impl
  
  using Literal = std::shared_ptr<impl::Base>;

  
  template <typename T> requires std::derived_from<T, types::Type>
  auto cast(Literal const &v) {
    if constexpr (std::is_same_v<T, types::IntegerType>) {
      return impl::cast<impl::Integer>(v);
    }
    else if constexpr (std::is_same_v<T, types::ArrayLike>) {
      return impl::cast<impl::ArrayLike>(v);
    }    
    else if constexpr (std::is_same_v<T, types::ArrayType>) {
      return impl::cast<impl::array>(v);
    }
    else if constexpr (std::is_same_v<T, types::StringType>) {
      return impl::cast<impl::string>(v);
    }
    else if constexpr (std::is_same_v<T, types::StructType>) {
      return impl::cast<impl::structT>(v);
    }
    else {
      static_assert(false, "invalid cast type");
    }
  }
  
  // Factories for each of the values
  inline Literal i8(int val) {
    return std::make_shared<impl::i8>(val);
  }

  inline Literal i16(int val) {
    return std::make_shared<impl::i16>(val);
  }

  inline Literal string(std::string const &str) {
    return std::make_shared<impl::string>(str);
  }

  template <typename ... Values>
  inline Literal structT(types::TypeHandle structType, Values&& ... values) {
    return std::make_shared<impl::structT>(structType, std::forward<Values>(values)...);
  }

  
  template <typename ... Elements>
  inline Literal array(types::TypeHandle elementType, Elements&& ... elems) {
    return std::make_shared<impl::array>(elementType, std::forward<Elements>(elems)...);
  }

  
} // namespace values

