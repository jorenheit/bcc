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


} // namespace proxy
