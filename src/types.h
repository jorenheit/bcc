#pragma once
#include <memory>
#include <vector>

namespace types {

  struct Type {
    virtual ~Type() = default;

    virtual int size() const = 0;
    virtual bool usesValue1() const = 0;
    virtual bool isInteger() const = 0;
    virtual bool isArray() const = 0;
    virtual bool isStruct() const = 0;
    virtual bool isPointer() const = 0;
  };

  struct VoidType : Type {
    inline int size() const override { return 0; }
    inline bool usesValue1() const override { return false; }
    inline virtual bool isInteger() const override { return false; }
    inline virtual bool isArray() const override { return false; }
    inline virtual bool isStruct() const override {return false; }
    inline virtual bool isPointer() const override {return false; }
  };

  struct IntegerType : Type {
    int bits;
    inline IntegerType(int bits_): bits(bits_) {}
    inline int size() const override { return 1; }
    inline bool usesValue1() const override { return bits > 8; }
    inline virtual bool isInteger() const override { return true; }
    inline virtual bool isArray() const override { return false; }
    inline virtual bool isStruct() const override {return false; }
    inline virtual bool isPointer() const override {return false; }
  };  

  struct ArrayType: Type {
    Type const *elementType;
    int length;

    inline ArrayType(Type const *elem, int len):  elementType(elem), length(len) {}
    inline int size() const override { return length * elementType->size(); }
    inline bool usesValue1() const override { return elementType->usesValue1(); }
    inline virtual bool isInteger() const override { return false; }
    inline virtual bool isArray() const override { return true; }
    inline virtual bool isStruct() const override {return false; }
    inline virtual bool isPointer() const override {return false; }
  };

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
    
    inline Type const *voidT() const { return _void.get(); }
    inline Type const *i8() const    { return _i8.get(); }
    inline Type const *i16() const   { return _i16.get(); }
    inline Type const *array(Type const* elem, int length) const {
      for (auto const &ptr: _arrayTypes) {
	if (ptr->elementType == elem && ptr->length == length) return ptr.get();
      }
      
      _arrayTypes.emplace_back(std::make_unique<ArrayType>(elem, length));
      return _arrayTypes.back().get();
    }
  };  
  
  using TypePtr = Type const *;

  template <typename T>
  T const *cast(Type const *ptr) { return static_cast<T const *>(ptr); }
  
  // TODO: implicit conversion I16 -> I8
  
}
