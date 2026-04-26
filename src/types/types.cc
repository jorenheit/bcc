#include "acus/types/types.h"
using namespace acus;

std::unique_ptr<types::VoidType> TypeSystem::_void;
std::unique_ptr<types::IntegerType> TypeSystem::_i8;
std::unique_ptr<types::IntegerType> TypeSystem::_i16;

std::vector<std::unique_ptr<types::RawType>> TypeSystem::_rawTypes;    
std::vector<std::unique_ptr<types::ArrayType>> TypeSystem::_arrayTypes;
std::vector<std::unique_ptr<types::StringType>> TypeSystem::_stringTypes;
std::vector<std::unique_ptr<types::StructType>> TypeSystem::_structTypes;
std::vector<std::unique_ptr<types::PointerType>> TypeSystem::_pointerTypes;
std::vector<std::unique_ptr<types::FunctionType>> TypeSystem::_functionTypes;

void TypeSystem::init() {
  static bool initialized = false;
  if (!initialized) {
    _void = std::make_unique<types::VoidType>();
    _i8 = std::make_unique<types::IntegerType>(8);
    _i16 = std::make_unique<types::IntegerType>(16);
    initialized = true;
  }
}
