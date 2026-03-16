#pragma once

namespace types {

  enum TypeEnum {
    I8, I16, ARRAY, STRUCT
  };
  
  struct Type {
    virtual int size() const  = 0;
    virtual bool usesValue1() const = 0;
    virtual TypeEnum getType() const = 0;
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

  template <typename T>
  struct Array: Type {
    int _size;

    Array(int sz): _size(sz) {}
    
    virtual int size() const override { return _size; }
    virtual bool usesValue1() const override { return T::usesValue1(); }
    virtual TypeEnum getType() const override { return ARRAY; }
  };

  inline bool match(Type const &t1, Type const &t2) {
    return t1.getType() == t2.getType();
  }
  
}
