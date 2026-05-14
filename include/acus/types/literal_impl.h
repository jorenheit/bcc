// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <unordered_map>
#include <memory>

#include "acus/types/types_fwd.h"

#define API_HEADER
#include "acus/api/api.h"


// Implementation of literal types (literal_impl.cc)
namespace acus::literal::impl {

  struct Base {
    types::TypeHandle _type;
    Base(types::TypeHandle t);

    virtual ~Base() = default;
    virtual std::shared_ptr<Base> clone() const = 0;
    virtual std::string str() const = 0;
    virtual types::TypeHandle type() const;
  };

  using Literal = std::shared_ptr<Base>;
    
  struct Integer: Base {
    int const _semanticValue;
    Integer(types::TypeHandle t, int v);

    virtual std::string str() const override;
    unsigned encodedValue() const;
    int semanticValue() const;
  };
    
  struct u8: Integer {
    u8(u8 const &other) = default;
    u8(int v, API_CTX_IGNORE);
    virtual Literal clone() const override;
  };

  struct s8: Integer {
    s8(s8 const &other) = default;
    s8(int v, API_CTX_IGNORE);
    virtual Literal clone() const override;
  };
  
  struct u16: Integer {
    u16(u16 const& other) = default;
    u16(int v, API_CTX_IGNORE);
    virtual Literal clone() const override;
  };      

  struct s16: Integer {
    s16(s16 const& other) = default;
    s16(int v, API_CTX_IGNORE);
    virtual Literal clone() const override;
  };      
  
  struct ArrayLike: Base {
    std::vector<Literal> arr;

    ArrayLike(types::TypeHandle type);
    Literal element(size_t idx) const;
  };
    
  struct string: ArrayLike {
    std::string const _str;

    string(std::string const &s, API_CTX);      
    string(string const &other);
    virtual std::string str() const override;      
    virtual Literal clone() const override;
    Literal element(size_t idx) const;
    std::string const &stdstr() const;
  }; // string

  struct structT: Base {

    std::vector<std::pair<std::string, Literal>> _fields;
    
    structT(types::TypeHandle type, std::unordered_map<std::string, Literal> const &fields, API_CTX);
    structT(structT const &other);      
    Literal field(std::string const &name) const;
    Literal field(size_t idx) const;      
    virtual Literal clone() const override;
    virtual std::string str() const override;
  }; // structT

  struct array: ArrayLike {
    types::TypeHandle elementType;

    array(types::TypeHandle elementType, std::vector<Literal> const &elements, API_CTX);      
    array(array const &other);

    virtual Literal clone() const override;
    virtual std::string str() const override;
  }; // array

    
  struct FunctionPointer: Base {
    std::string _functionName;
      
    FunctionPointer(types::FunctionType const *functionType, std::string const &fname, API_CTX_IGNORE);
    FunctionPointer(FunctionPointer const &other) = default;
      
    virtual std::shared_ptr<Base> clone() const override;      
    virtual std::string str() const override;
    std::string const &functionName() const;
  };
    
  template <typename V> requires std::derived_from<V, Base>
  auto cast(Literal const &v) {
    auto ptr = std::dynamic_pointer_cast<V>(v);
    assert(ptr != nullptr && "invalid value cast");
    return ptr;
  }
} // namespace literal::mpl

