#pragma once
#include <memory>

namespace types {

  enum TypeEnum {
    VOID, I8, I16, ARRAY, STRUCT
  };
  
  struct Type {
    virtual int size() const  = 0;
    virtual bool usesValue1() const = 0;
    virtual TypeEnum getType() const = 0;
  };

  using TypePtr = std::shared_ptr<Type>;

  struct Void: Type {
    virtual int size() const override { return 0; }
    virtual bool usesValue1() const override { return false; }
    virtual TypeEnum getType() const override { return VOID; }
  };
  
  struct i8: Type {
    virtual int size() const override { return 1; }
    virtual bool usesValue1() const override { return false; }
    virtual TypeEnum getType() const override { return I8; }
  };
  
  struct i16: Type {
    virtual int size() const override { return 1; }
    virtual bool usesValue1() const override { return true; }
    virtual TypeEnum getType() const override { return I16; }
  };

  template <int ElementWidth>
  struct Array: Type {
    static_assert(ElementWidth == 1 || ElementWidth == 2);
    int _size;

    Array(int sz): _size(sz) {}
    
    virtual int size() const override { return _size; }
    virtual bool usesValue1() const override { return ElementWidth == 1 ? 0 : 1; }
    virtual TypeEnum getType() const override { return ARRAY; }
  };


  inline bool match(TypePtr const &t1, TypePtr const &t2) {
    return t1->getType() == t2->getType() && t1->size() == t2->size();
  }

  template <typename T>
  bool match(TypePtr const &t) {
    return dynamic_cast<T const *>(t.get()) != nullptr;
  }

  inline bool isInteger(TypePtr const &t) {
    return match<i8>(t) || match<i16>(t);
  }

  // TODO: implicit conversion I16 -> I8
  
}
