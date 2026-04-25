#pragma once
#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <optional>
#include <variant>
#include "bfc/core/slot.h"
#include "bfc/types/values.h"

class Compiler;


namespace proxy {

  namespace Impl {
    class Base;
    using BasePtr = std::shared_ptr<Base>;

    struct SlotProxy: public BasePtr {
      SlotProxy(BasePtr ptr): BasePtr(std::move(ptr)) {}
      SlotProxy(Slot const &slot);
    };

    // TODO: all members are const now, even when they semantically aren't const
    // (modify the represented slot). Should they become non-const?
    class Base {
      types::TypeHandle _type;
    public:
      Base(types::TypeHandle t): _type(t) {}
      virtual ~Base() = default;

      types::TypeHandle type() const { return _type; }
      virtual std::string name() const = 0;
      virtual Slot materialize(Compiler &c) const = 0;
      virtual void write(Compiler &c, SlotProxy src) const = 0;
      virtual void write(Compiler &c, values::Literal src) const = 0;
      virtual Slot addressOf(Compiler &c) const = 0;
      virtual bool direct() const = 0;
    };
    

    class Direct: public Base {
      Slot _slot;
    
    public:
      Direct(Slot const &slot): Base(slot.type), _slot(slot) {}
      virtual Slot materialize(Compiler&) const override;
      virtual void write(Compiler &c, SlotProxy src) const override;
      virtual void write(Compiler &c, values::Literal src) const override;      
      virtual bool direct() const override { return true; }
      virtual Slot addressOf(Compiler &c) const override;

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
      
      virtual Slot materialize(Compiler &c) const override {
	return std::holds_alternative<int>(_index)
	  ? materializeImpl(c, std::get<int>(_index))
	  : materializeImpl(c, std::get<SlotProxy>(_index));
      }

      virtual void write(Compiler &c, SlotProxy src) const override {
	return std::holds_alternative<int>(_index)
	  ? writeImpl(c, std::get<int>(_index), src)
	  : writeImpl(c, std::get<SlotProxy>(_index), src);
      }

      virtual void write(Compiler &c, values::Literal src) const override {
	return std::holds_alternative<int>(_index)
	  ? writeImpl(c, std::get<int>(_index), src)
	  : writeImpl(c, std::get<SlotProxy>(_index), src);
      }

      virtual Slot addressOf(Compiler &c) const override;

      
    private:
      Slot materializeImpl(Compiler &c, int index) const;
      Slot materializeImpl(Compiler &c, SlotProxy index) const;

      void writeImpl(Compiler &c, int index, SlotProxy src) const;
      void writeImpl(Compiler &c, int index, values::Literal) const;
      void writeImpl(Compiler &c, SlotProxy index, SlotProxy src) const;
      void writeImpl(Compiler &c, SlotProxy index, values::Literal) const;

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

      virtual Slot materialize(Compiler &c) const override;
      virtual void write(Compiler &c, SlotProxy src) const override;
      virtual void write(Compiler &c, values::Literal src) const override;
      virtual Slot addressOf(Compiler &c) const override;

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
      
      virtual Slot materialize(Compiler &c) const;
      virtual void write(Compiler &c, SlotProxy src) const;;
      virtual void write(Compiler &c, values::Literal src) const;
      virtual Slot addressOf(Compiler &c) const override;      
      virtual bool direct() const { return false; }
    };
    

    
  } // namespace Impl
      
  using SlotProxy = Impl::SlotProxy;
  
  inline SlotProxy direct(Slot const &slot) {
    return SlotProxy(std::make_shared<Impl::Direct>(slot));
  }
  
  inline SlotProxy arrayElement(SlotProxy arr, int index) {
    return SlotProxy(std::make_shared<Impl::ArrayElement>(std::move(arr), index));
  }

  inline SlotProxy arrayElement(SlotProxy arr, SlotProxy index) {
    return SlotProxy(std::make_shared<Impl::ArrayElement>(std::move(arr), std::move(index)));
  }  

  inline SlotProxy structField(SlotProxy obj, std::string const &fieldName) {
    return SlotProxy(std::make_shared<Impl::StructField>(std::move(obj), fieldName));
  }

  inline SlotProxy dereferencedPointer(SlotProxy ptr) {
    return SlotProxy(std::make_shared<Impl::DereferencedPointer>(std::move(ptr)));
  }
  
  
} // namespace proxy
  
using SlotProxy = proxy::Impl::SlotProxy;

  
