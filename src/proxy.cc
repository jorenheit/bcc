#include "proxy.h"
#include "compiler.h"

namespace proxy {

  Impl::SlotProxy::SlotProxy(Slot const &slot):
    BasePtr(proxy::direct(slot))
  {}

  Slot Impl::Direct::materialize(Compiler&) const {
    return _slot;
  }
  
  void Impl::Direct::write(Compiler &c, SlotProxy src) const {
    c.assignSlot(_slot, src->materialize(c));
  }

  void Impl::Direct::write(Compiler &c, values::Anonymous src) const {
    c.assignSlot(_slot, src);
  }

  Slot Impl::ArrayElement::getElementSlot(Slot const &arrSlot, int index) const {
    return Slot {
      .name = std::string("__element::") + arrSlot.name + "[" + std::to_string(index) + "]",
      .type = this->type(),
      .kind = Slot::Dummy,
      .offset = arrSlot.offset + (index * this->type()->size())
    };
  }

  // Materialize a slot at known offset
  Slot Impl::ArrayElement::materializeImpl(Compiler &c, int index) const {
    return getElementSlot(_arr->materialize(c), index);
  }

  // Materialize a slot at unknown offset
  Slot Impl::ArrayElement::materializeImpl(Compiler &c, SlotProxy index) const {
    Slot const arrSlot = _arr->materialize(c);
    Slot const indexSlot = index->materialize(c);
    Slot const elementSlot = c.getTemp(this->type());

    c.copyElementIntoSlot(elementSlot, arrSlot, indexSlot);
    return elementSlot;
  }

  // Write an anonymous value to a slot at known offset
  void Impl::ArrayElement::writeImpl(Compiler &c, int index, values::Anonymous src) const {
    Slot const arrSlot = _arr->materialize(c);
    Slot const elementSlot = getElementSlot(arrSlot, index);

    c.assignSlot(elementSlot, src);
    if (not _arr->direct()) {
      _arr->write(c, arrSlot);
    }
  }

  // Write a slot-proxy to a slot at known offset
  void Impl::ArrayElement::writeImpl(Compiler &c, int index, SlotProxy src) const {
    Slot const srcSlot = src->materialize(c);
    Slot const arrSlot = _arr->materialize(c);
    Slot const elementSlot = getElementSlot(arrSlot, index);

    c.assignSlot(elementSlot, srcSlot);
    if (not _arr->direct()) {
      _arr->write(c, arrSlot);
    }
  }

  // Write an anonymous value to a dynamic offset
  void Impl::ArrayElement::writeImpl(Compiler &c, SlotProxy index, values::Anonymous src) const {
    writeImpl(c, index, c.getTemp(src));
  }

  // Write a slot-proxy to a dynamic offset
  void Impl::ArrayElement::writeImpl(Compiler &c, SlotProxy index, SlotProxy src) const {
    Slot const srcSlot = src->materialize(c);
    Slot const arrSlot = _arr->materialize(c);
    Slot const indexSlot = index->materialize(c);
    c.copySlotIntoElement(srcSlot, arrSlot, indexSlot);
    if (not _arr->direct()) {
      _arr->write(c, arrSlot);
    }
  }

  Impl::StructField::StructField(SlotProxy obj, std::string fieldName):
    Base(types::cast<types::StructType>(obj->type())->fieldType(fieldName)),
    _obj(obj),
    _fieldIndex(types::cast<types::StructType>(obj->type())->fieldIndex(fieldName)),
    _fieldOffset(types::cast<types::StructType>(obj->type())->fieldOffset(fieldName)),
    _fieldName(fieldName)
  {}
  
  Slot Impl::StructField::getFieldSlot(Slot const obj) const {
    auto structType = static_cast<types::StructType const *>(_obj->type());
    return Slot {
      .name = std::string("__field_") + name(),
      .type = structType->_fields[_fieldIndex].type,
      .kind = Slot::StructField,
      .offset = obj.offset + _fieldOffset
    };  
  }
  
  
  Slot Impl::StructField::materialize(Compiler &c) const {
    return getFieldSlot(_obj->materialize(c));
  }

  // Write an anonymous value to a slot at known offset
  void Impl::StructField::write(Compiler &c, values::Anonymous src) const {
    Slot const objSlot = _obj->materialize(c);
    Slot const fieldSlot = getFieldSlot(objSlot);

    c.assignSlot(fieldSlot, src);
    if (not _obj->direct()) {
      _obj->write(c, objSlot);
    }
  }

  // Write a slot-proxy to a slot at known offset
  void Impl::StructField::write(Compiler &c, SlotProxy src) const {
    Slot const srcSlot = src->materialize(c);
    Slot const objSlot = _obj->materialize(c);
    Slot const fieldSlot = getFieldSlot(objSlot);

    c.assignSlot(fieldSlot, srcSlot);
    if (not _obj->direct()) {
      _obj->write(c, objSlot);
    }
  }
  
  Slot Impl::DereferencedPointer::materialize(Compiler &c) const {
    Slot const ptrSlot = _ptr->materialize(c);
    Slot const destSlot = c.getTemp(this->type());
    c.dereferencePointerIntoSlot(ptrSlot, destSlot);
    return destSlot;
  }

  void Impl::DereferencedPointer::write(Compiler &c, SlotProxy src) const {
    assert(false && "writing to deref not impemented yet");
  }

  void Impl::DereferencedPointer::write(Compiler &c, values::Anonymous src) const {
    assert(false && "writing to deref not impemented yet");
  }

  
} // namespace proxy
