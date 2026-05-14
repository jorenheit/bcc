// Acus - A C++ library for generating Brainfuck programs.
// Copyright (C) 2026 Joren Heit
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <optional>
#include <variant>
#include "acus/core/slot.h"
#include "acus/types/literal.h"

namespace acus {
  class Builder;
}

namespace acus::proxy {

  namespace impl {
    class Base;
    using BasePtr = std::shared_ptr<Base>;

    struct SlotProxy: public BasePtr {
      SlotProxy(BasePtr ptr): BasePtr(std::move(ptr)) {}
      SlotProxy(Slot const &slot);
    };

    class Base {
      types::TypeHandle _type;
    public:
      Base(types::TypeHandle t): _type(t) {}
      virtual ~Base() = default;

      types::TypeHandle type() const { return _type; }
      virtual std::string name() const = 0;
      virtual Slot materialize(Assembler &a) const = 0;
      virtual void write(Assembler &a, SlotProxy src) const = 0;
      virtual void write(Assembler &a, acus::literal::Literal src) const = 0;
      virtual Slot addressOf(Assembler &a) const = 0;
      virtual bool direct() const = 0;
    };
    

    class Direct: public Base {
      Slot _slot;
    
    public:
      Direct(Slot const &slot): Base(slot.type), _slot(slot) {}
      virtual Slot materialize(Assembler&) const override;
      virtual void write(Assembler &a, SlotProxy src) const override;
      virtual void write(Assembler &a, acus::literal::Literal src) const override;      
      virtual bool direct() const override { return true; }
      virtual Slot addressOf(Assembler &a) const override;

      virtual std::string name() const override {
	return _slot.name;
      }
      
    };

    class ArrayElement: public Base {
      SlotProxy _arr;
      std::variant<int, SlotProxy> _index;
    
    public:
      ArrayElement(SlotProxy arr, int index):
	Base(cast<types::ArrayLike>(arr->type())->elementType()),
	_arr(std::move(arr)),
	_index(index)
      {}
      
      ArrayElement(SlotProxy arr, SlotProxy index):
	Base(cast<types::ArrayLike>(arr->type())->elementType()),
	_arr(std::move(arr)),
	_index(std::move(index))
      {}

      virtual bool direct() const override {
	return _arr->direct() && std::holds_alternative<int>(_index);
      }
      
      virtual std::string name() const override {
	std::string idx = std::holds_alternative<int>(_index)
	  ? std::to_string(std::get<int>(_index))
	  : std::get<SlotProxy>(_index)->name();
	return _arr->name() + "[" + idx + "]";
      }
      
      virtual Slot materialize(Assembler &a) const override {
	return std::holds_alternative<int>(_index)
	  ? materializeImpl(a, std::get<int>(_index))
	  : materializeImpl(a, std::get<SlotProxy>(_index));
      }

      virtual void write(Assembler &a, SlotProxy src) const override {
	return std::holds_alternative<int>(_index)
	  ? writeImpl(a, std::get<int>(_index), src)
	  : writeImpl(a, std::get<SlotProxy>(_index), src);
      }

      virtual void write(Assembler &a, acus::literal::Literal src) const override {
	return std::holds_alternative<int>(_index)
	  ? writeImpl(a, std::get<int>(_index), src)
	  : writeImpl(a, std::get<SlotProxy>(_index), src);
      }

      virtual Slot addressOf(Assembler &a) const override;

      
    private:
      Slot materializeImpl(Assembler &a, int index) const;
      Slot materializeImpl(Assembler &a, SlotProxy index) const;

      void writeImpl(Assembler &a, int index, SlotProxy src) const;
      void writeImpl(Assembler &a, int index, acus::literal::Literal) const;
      void writeImpl(Assembler &a, SlotProxy index, SlotProxy src) const;
      void writeImpl(Assembler &a, SlotProxy index, acus::literal::Literal) const;

      Slot getElementSlot(Slot const &arrSlot, int index) const;
    }; // ArrayElement

    class StructField: public Base {
      SlotProxy _obj;
      int _fieldIndex;
      int _fieldOffset;
      std::string _fieldName;

    public:
      StructField(SlotProxy obj, std::string fieldName);
      
      virtual bool direct() const override {
	return _obj->direct();
      }
      
      virtual std::string name() const override {
	return _obj->name() + "." + _fieldName;
      }

      virtual Slot materialize(Assembler &a) const override;
      virtual void write(Assembler &a, SlotProxy src) const override;
      virtual void write(Assembler &a, acus::literal::Literal src) const override;
      virtual Slot addressOf(Assembler &a) const override;

    private:
      Slot getFieldSlot(Slot const obj) const;
      
    };

    class DereferencedPointer: public Base {
      SlotProxy _ptr;

    public:
      DereferencedPointer(SlotProxy ptr):
	Base(types::cast<types::PointerType>(ptr->type())->pointeeType()),
	_ptr(std::move(ptr))
      {}

      virtual std::string name() const {
	return std::string("deref<") + _ptr->name() + ">";
      }
      
      virtual Slot materialize(Assembler &a) const;
      virtual void write(Assembler &a, SlotProxy src) const;;
      virtual void write(Assembler &a, acus::literal::Literal src) const;
      virtual Slot addressOf(Assembler &a) const override;      
      virtual bool direct() const { return false; }
    };
    

    
  } // namespace impl
      
  using SlotProxy = impl::SlotProxy;
  
  inline SlotProxy direct(Slot const &slot) {
    return SlotProxy(std::make_shared<impl::Direct>(slot));
  }
  
  inline SlotProxy arrayElement(SlotProxy arr, int index) {
    return SlotProxy(std::make_shared<impl::ArrayElement>(std::move(arr), index));
  }

  inline SlotProxy arrayElement(SlotProxy arr, SlotProxy index) {
    return SlotProxy(std::make_shared<impl::ArrayElement>(std::move(arr), std::move(index)));
  }  

  inline SlotProxy structField(SlotProxy obj, std::string const &fieldName) {
    return SlotProxy(std::make_shared<impl::StructField>(std::move(obj), fieldName));
  }

  inline SlotProxy dereferencedPointer(SlotProxy ptr) {
    return SlotProxy(std::make_shared<impl::DereferencedPointer>(std::move(ptr)));
  }
  
} // namespace acus::proxy
  
namespace acus {
  using SlotProxy = proxy::impl::SlotProxy;
}
  
