#pragma once
#include <tuple>
#include <vector>
#include "types.h"

// ============================================================
// Function Signature
// ============================================================

struct FunctionParameter {
  std::string name;
  types::TypePtr type;
};

struct FunctionSignature {
  types::TypePtr returnType;
  std::vector<FunctionParameter> params;

  template <typename ... Args>
  explicit FunctionSignature(types::TypePtr ret, Args&&...args):
    returnType(ret)
  {
    static_assert(sizeof ... (Args) % 2 == 0);
    static constexpr int pairCount = sizeof ... (Args) / 2;
    params.reserve(pairCount);
    
    auto allArgs = std::make_tuple(std::forward<Args>(args)...);    
    [&]<std::size_t... I>(std::index_sequence<I...>) {
      (params.emplace_back(FunctionParameter{
	  .name = std::get<2 * I>(allArgs),
	  .type = std::get<2 * I + 1>(allArgs)	  
	}), ...);
    }(std::make_index_sequence<pairCount>{});
  }
};

