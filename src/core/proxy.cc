#include "acus/core/proxy.h"
#include "acus/builder/builder.h"

namespace acus::proxy {

  impl::SlotProxy::SlotProxy(Slot const &slot):
    BasePtr(proxy::direct(slot))
  {}

  Slot impl::Direct::materialize(Builder&) const {
    return _slot;
  }
  
  void impl::Direct::write(Builder &c, SlotProxy src) const {
    c.assignSlot(_slot, src->materialize(c));
  }

  void impl::Direct::write(Builder &c, literal::Literal src) const {
    c.assignSlot(_slot, src);
  }

  Slot impl::Direct::addressOf(Builder &c) const {
    return c.addressOfSlot(_slot);
  }
  
  Slot impl::ArrayElement::getElementSlot(Slot const &arrSlot, int index) const {
    return arrSlot.sub(this->type(), index * this->type()->size());
  }

  // Materialize a slot at known offset
  Slot impl::ArrayElement::materializeImpl(Builder &c, int index) const {
    return getElementSlot(_arr->materialize(c), index);
  }

  // Materialize a slot at unknown offset
  Slot impl::ArrayElement::materializeImpl(Builder &c, SlotProxy index) const {
    Slot const arrSlot = _arr->materialize(c);
    Slot const indexSlot = index->materialize(c);
    Slot const elementSlot = c.getTemp(this->type());

    c.copyElementIntoSlot(elementSlot, arrSlot, indexSlot);
    return elementSlot;
  }

  // Write an anonymous value to a slot at known offset
  void impl::ArrayElement::writeImpl(Builder &c, int index, literal::Literal src) const {
    Slot const arrSlot = _arr->materialize(c);
    Slot const elementSlot = getElementSlot(arrSlot, index);

    c.assignSlot(elementSlot, src);
    if (not _arr->direct()) {
      _arr->write(c, arrSlot);
    }
  }

  // Write a slot-proxy to a slot at known offset
  void impl::ArrayElement::writeImpl(Builder &c, int index, SlotProxy src) const {
    Slot const srcSlot = src->materialize(c);
    Slot const arrSlot = _arr->materialize(c);
    Slot const elementSlot = getElementSlot(arrSlot, index);

    c.assignSlot(elementSlot, srcSlot);
    if (not _arr->direct()) {
      _arr->write(c, arrSlot);
    }
  }

  // Write an anonymous value to a dynamic offset
  void impl::ArrayElement::writeImpl(Builder &c, SlotProxy index, literal::Literal src) const {
    writeImpl(c, index, c.getTemp(src));
  }

  // Write a slot-proxy to a dynamic offset
  void impl::ArrayElement::writeImpl(Builder &c, SlotProxy index, SlotProxy src) const {
    Slot const srcSlot = src->materialize(c);
    Slot const arrSlot = _arr->materialize(c);
    Slot const indexSlot = index->materialize(c);
    c.copySlotIntoElement(srcSlot, arrSlot, indexSlot);
    if (not _arr->direct()) {
      _arr->write(c, arrSlot);
    }
  }

  Slot impl::ArrayElement::addressOf(Builder &c) const {
    Slot ptr = _arr->addressOf(c);
    ptr.type = ts::pointer(this->type());
    if (std::holds_alternative<int>(_index)) {
      c.addAssign(ptr, literal::i16(std::get<int>(_index)));
    } else {
      c.addAssign(ptr, std::get<SlotProxy>(_index));
    }
    return ptr;
  }
  
  impl::StructField::StructField(SlotProxy obj, std::string fieldName):
    Base(types::cast<types::StructType>(obj->type())->fieldType(fieldName)),
    _obj(obj),
    _fieldIndex(types::cast<types::StructType>(obj->type())->fieldIndex(fieldName)),
    _fieldOffset(types::cast<types::StructType>(obj->type())->fieldOffset(fieldName)),
    _fieldName(fieldName)
  {}
  
  Slot impl::StructField::getFieldSlot(Slot const obj) const {
    auto structType = static_cast<types::StructType const *>(_obj->type());
    return Slot {
      .name = std::string("__field_") + name(),
      .type = structType->_fields[_fieldIndex].type,
      .kind = Slot::StructField,
      .offset = obj.offset + _fieldOffset
    };  
  }
  
  
  Slot impl::StructField::materialize(Builder &c) const {
    return getFieldSlot(_obj->materialize(c));
  }

  // Write an anonymous value to a slot at known offset
  void impl::StructField::write(Builder &c, literal::Literal src) const {
    Slot const objSlot = _obj->materialize(c);
    Slot const fieldSlot = getFieldSlot(objSlot);

    c.assignSlot(fieldSlot, src);
    if (not _obj->direct()) {
      _obj->write(c, objSlot);
    }
  }

  // Write a slot-proxy to a slot at known offset
  void impl::StructField::write(Builder &c, SlotProxy src) const {
    Slot const srcSlot = src->materialize(c);
    Slot const objSlot = _obj->materialize(c);
    Slot const fieldSlot = getFieldSlot(objSlot);

    c.assignSlot(fieldSlot, srcSlot);
    if (not _obj->direct()) {
      _obj->write(c, objSlot);
    }
  }

  Slot impl::StructField::addressOf(Builder &c) const {
    Slot ptr = _obj->addressOf(c);
    ptr.type = ts::pointer(this->type());
    c.addAssign(ptr, literal::i16(_fieldOffset));
    return ptr;
  }
  
  Slot impl::DereferencedPointer::materialize(Builder &c) const {
    Slot const ptrSlot = _ptr->materialize(c);
    Slot const destSlot = c.getTemp(this->type());
    c.dereferencePointerIntoSlot(ptrSlot, destSlot);
    return destSlot;
  }

  void impl::DereferencedPointer::write(Builder &c, SlotProxy src) const {
    Slot const ptrSlot = _ptr->materialize(c);
    Slot const srcSlot = src->materialize(c);
    c.writeSlotThroughDereferencedPointer(ptrSlot, srcSlot);
  }

  void impl::DereferencedPointer::write(Builder &c, literal::Literal src) const {
    Slot const ptrSlot = _ptr->materialize(c);
    Slot const srcSlot = c.getTemp(src);
    c.writeSlotThroughDereferencedPointer(ptrSlot, srcSlot);
  }

  Slot impl::DereferencedPointer::addressOf(Builder &c) const {
    return _ptr->materialize(c);
  }
  
} // namespace proxy
