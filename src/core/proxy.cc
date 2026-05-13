#include "acus/core/proxy.h"
#include "acus/assembler/assembler.h"

namespace acus::proxy {

  impl::SlotProxy::SlotProxy(Slot const &slot):
    BasePtr(proxy::direct(slot))
  {}

  Slot impl::Direct::materialize(Assembler &) const {
    return _slot;
  }
  
  void impl::Direct::write(Assembler &a, SlotProxy src) const {
    a.assignSlot(_slot, src->materialize(a));
  }

  void impl::Direct::write(Assembler &a, literal::Literal src) const {
    a.assignSlot(_slot, src);
  }

  Slot impl::Direct::addressOf(Assembler &a) const {
    return a.addressOfSlot(_slot);
  }
  
  Slot impl::ArrayElement::getElementSlot(Slot const &arrSlot, int index) const {
    return arrSlot.sub(this->type(), index * this->type()->size());
  }

  // Materialize a slot at known offset
  Slot impl::ArrayElement::materializeImpl(Assembler &a, int index) const {
    return getElementSlot(_arr->materialize(a), index);
  }

  // Materialize a slot at unknown offset
  Slot impl::ArrayElement::materializeImpl(Assembler &a, SlotProxy index) const {
    Slot const arrSlot = _arr->materialize(a);
    Slot const indexSlot = index->materialize(a);
    Slot const elementSlot = a.getTemp(this->type());

    a.copyElementIntoSlot(elementSlot, arrSlot, indexSlot);
    return elementSlot;
  }

  // Write an anonymous value to a slot at known offset
  void impl::ArrayElement::writeImpl(Assembler &a, int index, literal::Literal src) const {
    Slot const arrSlot = _arr->materialize(a);
    Slot const elementSlot = getElementSlot(arrSlot, index);

    a.assignSlot(elementSlot, src);
    if (not _arr->direct()) {
      _arr->write(a, arrSlot);
    }
  }

  // Write a slot-proxy to a slot at known offset
  void impl::ArrayElement::writeImpl(Assembler &a, int index, SlotProxy src) const {
    Slot const srcSlot = src->materialize(a);
    Slot const arrSlot = _arr->materialize(a);
    Slot const elementSlot = getElementSlot(arrSlot, index);

    a.assignSlot(elementSlot, srcSlot);
    if (not _arr->direct()) {
      _arr->write(a, arrSlot);
    }
  }

  // Write an anonymous value to a dynamic offset
  void impl::ArrayElement::writeImpl(Assembler &a, SlotProxy index, literal::Literal src) const {
    writeImpl(a, index, a.getTemp(src));
  }

  // Write a slot-proxy to a dynamic offset
  void impl::ArrayElement::writeImpl(Assembler &a, SlotProxy index, SlotProxy src) const {
    Slot const srcSlot = src->materialize(a);
    Slot const arrSlot = _arr->materialize(a);
    Slot const indexSlot = index->materialize(a);
    a.copySlotIntoElement(srcSlot, arrSlot, indexSlot);
    if (not _arr->direct()) {
      _arr->write(a, arrSlot);
    }
  }

  Slot impl::ArrayElement::addressOf(Assembler &a) const {
    Slot ptr = _arr->addressOf(a);
    ptr.type = ts::pointer(this->type());
    if (std::holds_alternative<int>(_index)) {
      a.addAssign(ptr, literal::i16(std::get<int>(_index)));
    } else {
      a.addAssign(ptr, std::get<SlotProxy>(_index));
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
      .kind = obj.kind == Slot::Temp ? Slot::Temp : Slot::StructField,
      .offset = obj.offset + _fieldOffset
    };  
  }
  
  
  Slot impl::StructField::materialize(Assembler &a) const {
    return getFieldSlot(_obj->materialize(a));
  }

  // Write an anonymous value to a slot at known offset
  void impl::StructField::write(Assembler &a, literal::Literal src) const {
    Slot const objSlot = _obj->materialize(a);
    Slot const fieldSlot = getFieldSlot(objSlot);

    a.assignSlot(fieldSlot, src);
    if (not _obj->direct()) {
      _obj->write(a, objSlot);
    }
  }

  // Write a slot-proxy to a slot at known offset
  void impl::StructField::write(Assembler &a, SlotProxy src) const {
    Slot const srcSlot = src->materialize(a);
    Slot const objSlot = _obj->materialize(a);
    Slot const fieldSlot = getFieldSlot(objSlot);

    a.assignSlot(fieldSlot, srcSlot);
    if (not _obj->direct()) {
      _obj->write(a, objSlot);
    }
  }

  Slot impl::StructField::addressOf(Assembler &a) const {
    Slot ptr = _obj->addressOf(a);
    ptr.type = ts::pointer(this->type());
    a.addAssign(ptr, literal::i16(_fieldOffset));
    return ptr;
  }
  
  Slot impl::DereferencedPointer::materialize(Assembler &a) const {
    Slot const ptrSlot = _ptr->materialize(a);
    Slot const destSlot = a.getTemp(this->type());
    a.dereferencePointerIntoSlot(ptrSlot, destSlot);
    return destSlot;
  }

  void impl::DereferencedPointer::write(Assembler &a, SlotProxy src) const {
    Slot const ptrSlot = _ptr->materialize(a);
    Slot const srcSlot = src->materialize(a);
    a.writeSlotThroughDereferencedPointer(ptrSlot, srcSlot);
  }

  void impl::DereferencedPointer::write(Assembler &a, literal::Literal src) const {
    Slot const ptrSlot = _ptr->materialize(a);
    Slot const srcSlot = a.getTemp(src);
    a.writeSlotThroughDereferencedPointer(ptrSlot, srcSlot);
  }

  Slot impl::DereferencedPointer::addressOf(Assembler &a) const {
    return _ptr->materialize(a);
  }
  
} // namespace proxy
