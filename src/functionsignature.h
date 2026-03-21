#pragma once
#include <tuple>
#include <vector>
#include "types.h"

// ============================================================
// Function Signature
// ============================================================

struct FunctionParam {
  std::string name;
  types::TypeHandle type;
};

struct FunctionSignature {
  types::TypeHandle returnType;
  std::vector<FunctionParam> params;

  template <typename ... Args>
  explicit FunctionSignature(types::TypeHandle ret, Args&&...args):
    returnType(ret)
  {
    static_assert(sizeof ... (Args) % 2 == 0);
    static constexpr int pairCount = sizeof ... (Args) / 2;
    params.reserve(pairCount);
    
    auto allArgs = std::make_tuple(std::forward<Args>(args)...);    
    [&]<std::size_t... I>(std::index_sequence<I...>) {
      (params.emplace_back(FunctionParam{
	  .name = std::get<2 * I>(allArgs),
	  .type = std::get<2 * I + 1>(allArgs)	  
	}), ...);
    }(std::make_index_sequence<pairCount>{});
  }
};

