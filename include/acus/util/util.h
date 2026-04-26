#pragma once

namespace acus::util {

  template <typename ... Offsets>
  inline bool allDifferent(Offsets ... offsets) {
    static constexpr int N = sizeof...(Offsets);
    int const array[N] = { offsets... };
    for (int i = 0 ; i != N - 1; ++i) {
      for (int j = i + 1; j != N; ++j) {
	if (array[i] == array[j]) return false;
      }
    }
    return true;
  }

}
