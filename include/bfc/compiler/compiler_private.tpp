#pragma once

template <auto FetchOrPut>
void Compiler::syncGlobal(Slot const &localSlot, bool onlyAliasedGlobals) {
  assert(localSlot.kind == Slot::GlobalReference);
  
  std::string globalName = localSlot.name.substr(std::string("__g_").size());
  assert(_program.isGlobal(globalName));
  if (onlyAliasedGlobals && not _aliasedGlobals.contains(globalName)) return;
  
  Slot const &globalSlot = _program.globalSlot(globalName);
  assert(globalSlot.size() == localSlot.size());

  (this->*FetchOrPut)(globalSlot, localSlot);
}

template <auto FetchOrPut>
void Compiler::syncGlobals(bool onlyAliasedGlobals) {
  auto const &locals = _currentFunction->frame.locals;
  for (auto const &localSlot: locals) {
    if (localSlot.kind != Slot::GlobalReference) continue;
    syncGlobal<FetchOrPut>(localSlot, onlyAliasedGlobals);
  }  
}

inline void Compiler::syncGlobalToLocal(bool onlyAliasedGlobals) {
  syncGlobals<&Compiler::fetchGlobal>(onlyAliasedGlobals);
}

inline void Compiler::syncLocalToGlobal(bool onlyAliasedGlobals) {
  syncGlobals<&Compiler::putGlobal>(onlyAliasedGlobals);
}

template <typename Primitive, typename ... Args>
void Compiler::emit(Args&& ... args) {
  assert(_currentSeq != nullptr);
  _currentSeq->emplace<Primitive>(std::forward<Args>(args)...);
}

template <typename... Args> requires ((std::convertible_to<Args, Cell>) && ...)
auto Compiler::getFieldIndices(Args... args) {
  return std::make_tuple(getFieldIndex(static_cast<Cell>(args))...);
}
