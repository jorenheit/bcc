#pragma once

namespace acus::literal::impl {

  struct Base {
    types::TypeHandle _type;
    Base(types::TypeHandle t): _type(t) {}
    virtual ~Base() = default;

    virtual types::TypeHandle type() const { return _type; }      
    virtual std::shared_ptr<Base> clone() const = 0;
    virtual std::string str() const = 0;
  };

  using Literal = std::shared_ptr<Base>;
    
  struct Integer: Base {
    int _value;
    Integer(types::TypeHandle t, int v): Base(t), _value(v) {}

    virtual std::string str() const override { return std::to_string(_value); }
    int value() const { return _value; }      
  };
    
  struct i8: Integer {
    i8(i8 const &other) = default;
    i8(int v, API_CTX_IGNORE): Integer(ts::i8(), v & 0xff) {}
    virtual Literal clone() const override { return std::make_shared<i8>(*this); }
  };

  struct i16: Integer {
    i16(i16 const& other) = default;
    i16(int v, API_CTX_IGNORE): Integer(ts::i16(), v & 0xffff) {}
    virtual Literal clone() const override { return std::make_shared<i16>(*this); }      
  };      

  struct ArrayLike: Base {
    std::vector<Literal> arr;

    ArrayLike(types::TypeHandle type):
      Base(type)
    {}

    Literal element(size_t idx) const {
      assert(idx < arr.size());
      return arr[idx];
    }	
  };
    
  struct string: ArrayLike {
    std::string const _str;

    template <typename StringType> requires std::constructible_from<std::string, StringType>    
    string(StringType const &s, API_CTX):
      ArrayLike(ts::string(std::string(s).length())),
      _str(s)
    {
      for (char c: _str)  (ArrayLike::arr).emplace_back(std::make_shared<i8>(c, API_FWD));
      (ArrayLike::arr).emplace_back(std::make_shared<i8>(0, API_FWD));
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
      
    virtual Literal clone() const override {
      return std::make_shared<string>(*this);
    }

    Literal element(size_t idx) const {
      assert(idx < _str.size() + 1);
      return arr[idx];
    }
  }; // string

  struct structT: Base {
    struct Field {
      std::string name;
      Literal value;
    };

    std::vector<Field> _fields;

    structT(types::TypeHandle type, std::vector<Literal> const &values, API_CTX):
      Base(type)
    {
      // Initialize the fields and check types
      auto structType = types::cast<types::StructType>(type);
      for (int i = 0; i != structType->fieldCount(); ++i) {
	API_EXPECT_TYPE(values[i]->type(), structType->fieldType(i));

	_fields.push_back({
	    .name = structType->fieldName(i),
	    .value = values[i]
	  });	  
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
      
    Literal field(std::string const &name) const {
      for (Field const &f: _fields) if (f.name == name) return f.value;
      assert(false && "invalid field name");
      std::unreachable();
    }

    Literal field(size_t idx) const {
      assert(idx < _fields.size() && "field index out of bounds");
      return _fields[idx].value;
    }      
      
    virtual Literal clone() const override {
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

    // TODO: accept arrayType, not elementType
    array(types::TypeHandle elementType, std::vector<Literal> const &elements, API_CTX):
      ArrayLike(ts::array(elementType, elements.size())),
      elementType(elementType)
    {
      for (auto const &value: elements) {
	API_EXPECT_TYPE(value->type(), elementType);
      }
	
      ArrayLike::arr = elements;
    }
      
      
    array(array const &other): ArrayLike(other.type()), elementType(other.elementType) {
      ArrayLike::arr.reserve(other.arr.size());
      for (auto const& elem: other.arr) {
	ArrayLike::arr.emplace_back(elem->clone());
      }
    }

    virtual Literal clone() const override {
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

    
  struct FunctionPointer: Base {
    std::string _functionName;
      
    FunctionPointer(types::FunctionType const *functionType, std::string const &fname, API_CTX_IGNORE):
      Base(ts::function_pointer(functionType)),
      _functionName(fname)
    {}

    FunctionPointer(FunctionPointer const &other) = default;
      
    virtual std::shared_ptr<Base> clone() const override {
      return std::make_shared<FunctionPointer>(*this);
    }
      
    virtual std::string str() const override {
      return "fptr<" + _functionName + ">";
    }

    std::string const &functionName() const {
      return _functionName;
    }
  };
    
  template <typename V> requires std::derived_from<V, Base>
  auto cast(Literal const &v) {
    auto ptr = std::dynamic_pointer_cast<V>(v);
    assert(ptr != nullptr && "invalid value cast");
    return ptr;
  }


  // Builder classes for arrays and structs
  class StructLiteral {
  public:
    Literal operator()(auto const&... args) && {
      std::vector<Literal> values = {args ...};
      return std::move(*this)(values);
    }

    Literal operator()(std::vector<Literal> const &values) && {
      _called = true;
      return std::make_shared<impl::structT>(_structType, values, API_FWD);
    }
  
    ~StructLiteral() noexcept(false) {
      API_REQUIRE(_called, "operator() must be called on result of literal::structT(); "
		  "e.g. values::structT(\"Point\")(...);");
    }

    StructLiteral(types::TypeHandle structType, api::impl::Context const &ctx):
      _structType(structType),
      API_CTX_NAME(ctx)
    {}
      
  private:
    types::TypeHandle _structType;
    api::impl::Context API_CTX_NAME;
    bool _called = false;
  
    StructLiteral(StructLiteral const&) = delete;
    StructLiteral(StructLiteral&&) = delete;
    StructLiteral& operator=(StructLiteral const&) = delete;
    StructLiteral& operator=(StructLiteral&&) = delete;
  };

  class ArrayLiteral {
  public:
    Literal operator()(auto const&... args) && {
      std::vector<Literal> values = {args ...};
      return std::move(*this)(values);
    }

    Literal operator()(std::vector<Literal> const &values) && {
      _called = true;
      return std::make_shared<impl::array>(_elementType, values, API_FWD);
    }
  
    ~ArrayLiteral() noexcept(false) {
      API_REQUIRE(_called, "operator() must be called on result of liteal::array(); "
		  "e.g. values::array(ts::i8())(...);");
    }

    ArrayLiteral(types::TypeHandle elementType, api::impl::Context const &ctx):
      _elementType(elementType),
      API_CTX_NAME(ctx)
    {}

  private:
    types::TypeHandle _elementType;
    api::impl::Context API_CTX_NAME;
    bool _called = false;
  
  
    ArrayLiteral(ArrayLiteral const&) = delete;
    ArrayLiteral(ArrayLiteral&&) = delete;
    ArrayLiteral& operator=(ArrayLiteral const&) = delete;
    ArrayLiteral& operator=(ArrayLiteral&&) = delete;
  };
  
} // namespace literal::mpl
