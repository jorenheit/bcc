#pragma once

template <auto FetchOrPut>
void Builder::syncGlobal(Slot const &localSlot, bool onlyAliasedGlobals) {
  assert(localSlot.kind == Slot::GlobalReference);
  
  std::string globalName = localSlot.name.substr(std::string("__g_").size());
  assert(_program.isGlobal(globalName));
  if (onlyAliasedGlobals && not _aliasedGlobals.contains(globalName)) return;
  
  Slot const &globalSlot = _program.globalSlot(globalName);
  assert(globalSlot.size() == localSlot.size());

  (this->*FetchOrPut)(globalSlot, localSlot);
}

template <auto FetchOrPut>
void Builder::syncGlobals(bool onlyAliasedGlobals) {
  auto const &locals = _currentFunction->frame.locals;
  for (auto const &localSlot: locals) {
    if (localSlot.kind != Slot::GlobalReference) continue;
    syncGlobal<FetchOrPut>(localSlot, onlyAliasedGlobals);
  }  
}

inline void Builder::syncGlobalToLocal(bool onlyAliasedGlobals) {
  syncGlobals<&Builder::fetchGlobal>(onlyAliasedGlobals);
}

inline void Builder::syncLocalToGlobal(bool onlyAliasedGlobals) {
  syncGlobals<&Builder::putGlobal>(onlyAliasedGlobals);
}


inline std::string Builder::defaultOpenTag() {
  static int count = 0;
  return std::string("open_loop_") + std::to_string(count++);
} 

inline std::string Builder::defaultCloseTag() {
  static int count = 0;
  return std::string("close_loop_") + std::to_string(count++);
}

inline int Builder::getFieldIndex(int offset, int field) {
  return offset * MacroCell::FieldCount + field;
}

inline int Builder::getFieldIndex(Cell cell) {
  return getFieldIndex(cell.offset, cell.field);
}

template <typename Primitive, typename ... Args>
void Builder::emit(Args&& ... args) {
  assert(_currentSeq != nullptr);
  _currentSeq->emplace<Primitive>(std::forward<Args>(args)...);
}

template <typename... Args> requires ((std::convertible_to<Args, Cell>) && ...)
auto Builder::getFieldIndices(Args... args) {
  return std::make_tuple(getFieldIndex(static_cast<Cell>(args))...);
}
