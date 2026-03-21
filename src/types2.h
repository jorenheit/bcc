#pragma once
#include <memory>
#include <vector>
#include <cassert>

namespace types {

  enum TypeEnum {
    VOID, I8, I16, ARRAY, STRUCT, POINTER
  };
  
  struct TypeInfo {
    virtual ~TypeInfo() = default;
    virtual TypeEnum typeEnum() const = 0;
    virtual int size() const = 0;
    virtual int length() const = 0;
    virtual bool usesValue1() const = 0;
    virtual bool isInteger() const = 0;
    virtual bool isArray() const = 0;
    virtual bool isStruct() const = 0;
    virtual bool isPointer() const = 0;
  };

  using TypeTag = TypeInfo const *;

  
  struct VoidType : TypeInfo {
    inline virtual TypeEnum typeEnum() const override { return VOID; }
    inline virtual int size() const override { return 0; }
    inline virtual int length() const { return 0; }
    inline virtual bool usesValue1() const override { return false; }
    inline virtual bool isInteger() const override { return false; }
    inline virtual bool isArray() const override { return false; }
    inline virtual bool isStruct() const override {return false; }
    inline virtual bool isPointer() const override {return false; }
  };

  struct IntegerType : TypeInfo {
    int bits;
    inline IntegerType(int bits_): bits(bits_) {}
    inline virtual TypeEnum typeEnum() const override { return bits > 8 ? I16 : I8; }
    inline virtual int size() const override { return 1; }
    inline virtual int length() const { return 1; }
    inline virtual bool usesValue1() const override { return bits > 8; }
    inline virtual bool isInteger() const override { return true; }
    inline virtual bool isArray() const override { return false; }
    inline virtual bool isStruct() const override {return false; }
    inline virtual bool isPointer() const override {return false; }
  };  

  struct ArrayType: TypeInfo {
    TypeTag elementType;
    int _length;

    inline ArrayType(TypeTag elem, int len):  elementType(elem), _length(len) {}
    inline virtual TypeEnum typeEnum() const override { return ARRAY; }
    inline virtual int size() const override { return _length * elementType->size(); }
    inline virtual int length() const { return _length; }
    inline virtual bool usesValue1() const override { return elementType->usesValue1(); }
    inline virtual bool isInteger() const override { return false; }
    inline virtual bool isArray() const override { return true; }
    inline virtual bool isStruct() const override {return false; }
    inline virtual bool isPointer() const override {return false; }
  };

  class TypeSystem;

  namespace Instance {
    template <bool Valid> struct valid_as_array_element {
      static constexpr bool is_valid_array_element = Valid;
    };
      
    struct Base {
      virtual TypeTag type(TypeSystem const &) const = 0;
      virtual Base const *element(size_t idx) const { assert(false); return nullptr; }
    };

    struct Var: Base, valid_as_array_element<false> {
      std::string varName;
      Var(Base const &base): varName(static_cast<Var const &>(base).varName) {}
      Var(std::string const &name): varName(name) {}
      virtual TypeTag type(TypeSystem const &ts) const override;
    };

    struct voidT: Base, valid_as_array_element<false> {
      virtual TypeTag type(TypeSystem const &ts) const override;
      static TypeTag staticType(TypeSystem const &);
    };

    struct i8 : Base, valid_as_array_element<true> {
      int const value;
      i8(Base const& base): value(static_cast<i8 const&>(base).value) {}
      i8(int v = 0) : value(v & 0xff) {}
      virtual TypeTag type(TypeSystem const &ts) const override;
    };

    struct i16 : Base, valid_as_array_element<true> {
      int const value;
      i16(Base const& base): value(static_cast<i16 const&>(base).value) {}
      i16(int v = 0) : value(v & 0xffff) {}
      virtual TypeTag type(TypeSystem const &ts) const override;
      static TypeTag staticType(TypeSystem const &ts);
    };      

    struct array : Base, valid_as_array_element<true> {
      TypeTag elementType;
      std::vector<std::unique_ptr<Base>> arr;

      template <typename... Values>
      array(TypeTag elementType, Values&&... values):
	elementType(elementType)
      {
	arr.reserve(sizeof...(Values));
	(arr.emplace_back(make_array_item(std::forward<Values>(values))), ...);
      }
      
      array(Base const& base) {
	auto const& other = static_cast<array const&>(base);
	arr.reserve(other.arr.size());
	for (auto const& elem : other.arr) {
	  arr.emplace_back(make_array_item(elem));
	}
      }

      virtual TypeTag type(TypeSystem const &ts) const override;
      virtual Base const *element(size_t idx) const override {
	assert(idx < arr.size());
	return arr[idx].get();
      }
    private:
      template <typename Arg>
      static std::unique_ptr<Base> make_array_item(Arg&& arg) {
	if constexpr (std::same_as<std::remove_cvref_t<Arg>, std::shared_ptr<Base>> ||
		      std::same_as<std::remove_cvref_t<Arg>, std::unique_ptr<Base>>) {
	  if (dynamic_cast<Var const*>(arg.get())) return std::make_unique<Var>(*arg);
	  switch (elementType) {
	  case VOID: assert(false);
	  case I8:  return std::make_unique<Instance::i8>(*static_cast<Instance::i8 const *>(arg.get()));
	  case I16: return std::make_unique<Instance::i16>(*static_cast<Instance::i16 const *>(arg.get()));
	  case ARRAY: return std::make_unique<Instance::array>(*static_cast<Instance::i16 const *>(arg.get()));
	  }
	} else if constexpr (std::constructible_from<std::string, Arg>) {
	  return std::make_unique<Var>(std::string(std::forward<Arg>(arg)));
	} else {
	  switch (elementType) {
	  case VOID: assert(false);
	  case I8:  return std::make_unique<Instance::i8>(std::forward<Arg>(arg));
	  case I16: return std::make_unique<Instance::i16>(std::forward<Arg>(arg));
	  case ARRAY: return std::make_unique<Instance::array>(std::forward<Arg>(arg));
	  }
	}
      }
    }; // array
  } // namespace instance

  template <typename ... Args> 
  std::shared_ptr<Base> instance(TypeTag type, Args&& ... args) const {
    switch (type->typeEnum()) {
    case VOID: assert(false);
    case I8:    return std::make_shared<Instance::i8>(std::forward<Args>(args)...);
    case I16:   return std::make_shared<Instance::i16>(std::forward<Args>(args)...);
    case ARRAY: return std::make_shared<Instance::array>(std::forward<Args>(args)...);
    };
  }

  std::shared_ptr<Base> var(std::string const &varName) {
    return std::make_shared<instance::Var>(varName);
  }

  
  class TypeSystem {
    
    std::unique_ptr<VoidType> _void;
    std::unique_ptr<IntegerType> _i8;
    std::unique_ptr<IntegerType> _i16;

    mutable std::vector<std::unique_ptr<ArrayType>> _arrayTypes;
    
  public:
    
    TypeSystem():
      _void(std::make_unique<VoidType>()),
      _i8(std::make_unique<IntegerType>(8)),
      _i16(std::make_unique<IntegerType>(16))
    {}

    TypeSystem(TypeSystem const &) = delete;
    TypeSystem &operator=(TypeSystem const &) = delete;
    
    inline TypeTag voidT() const { return _void.get(); }
    inline TypeTag i8() const    { return _i8.get(); }
    inline TypeTag i16() const   { return _i16.get(); }
    inline TypeTag array(TypeTag elem, int length) const {
      for (auto const &ptr: _arrayTypes) {
	if (ptr->elementType == elem && ptr->length() == length) return ptr.get();
      }
      _arrayTypes.emplace_back(std::make_unique<ArrayType>(elem, length));
      return _arrayTypes.back().get();
    }
  };  

  inline TypeTag instance::voidT::type(TypeSystem const &ts) const { return ts.voidT(); }
  inline TypeTag   instance::Var::type(TypeSystem const &ts) const { return nullptr; }
  inline TypeTag    instance::i8::type(TypeSystem const &ts) const { return ts.i8(); }
  inline TypeTag   instance::i16::type(TypeSystem const &ts) const { return ts.i16(); }
  inline TypeTag instance::array::type(TypeSystem const &ts) {
    return ts.array(elementType, arr.size());
  }

  
  template <typename T>
  T const *cast(TypeTag ptr) { return static_cast<T const *>(ptr); }
  

}
