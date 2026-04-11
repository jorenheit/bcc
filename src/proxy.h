#pragma once
#include <functional>
#include <optional>
#include <variant>
#include "slot.h"
#include "values.h"

class Compiler;


namespace proxy {

  namespace Impl {
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
      virtual Slot materialize(Compiler &c) const = 0;
      virtual void write(Compiler &c, SlotProxy src) const = 0;
      virtual void write(Compiler &c, values::Anonymous src) const = 0;
      virtual bool direct() const { return false; }
    };
    

  
    class Direct: public Base {
      Slot _slot;
    
    public:
      Direct(Slot const &slot): Base(slot.type), _slot(slot) {}
      virtual Slot materialize(Compiler&) const override;
      virtual void write(Compiler &c, SlotProxy src) const override;
      virtual void write(Compiler &c, values::Anonymous src) const override;      
      virtual bool direct() const override { return true; }

      virtual std::string name() const override {
	return _slot.name;
      }
      
    };

    class ArrayElement: public Base {
      SlotProxy _arr;
      std::variant<int, SlotProxy> _index;
    
    public:
      ArrayElement(SlotProxy arr, int index):
	Base(arr->type()->elementType()),
	_arr(std::move(arr)),
	_index(index)
      {}
      
      ArrayElement(SlotProxy arr, SlotProxy index):
	Base(arr->type()->elementType()),
	_arr(std::move(arr)),
	_index(std::move(index))
      {}

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

      virtual void write(Compiler &c, values::Anonymous src) const override {
	return std::holds_alternative<int>(_index)
	  ? writeImpl(c, std::get<int>(_index), src)
	  : writeImpl(c, std::get<SlotProxy>(_index), src);
      }
      
    private:
      Slot materializeImpl(Compiler &c, int index) const;
      Slot materializeImpl(Compiler &c, SlotProxy index) const;

      void writeImpl(Compiler &c, int index, SlotProxy src) const;
      void writeImpl(Compiler &c, int index, values::Anonymous) const;
      void writeImpl(Compiler &c, SlotProxy index, SlotProxy src) const;
      void writeImpl(Compiler &c, SlotProxy index, values::Anonymous) const;

      Slot getElementSlot(Slot const &arrSlot, int index) const;
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
    
} // namespace proxy
  
using SlotProxy = proxy::Impl::SlotProxy;

  
