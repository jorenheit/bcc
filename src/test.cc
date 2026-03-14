#include <iostream>

namespace Algorithm {

  std::string movePtr(int current, int dest) {
    char const ch = (current < dest) ? '>' : '<';
    int amount = std::abs(dest - current);
    return std::string(amount, ch);
  }

  std::string decrement(int n) { return std::string(n, '-'); }
  std::string increment(int n) { return std::string(n, '+'); }


  template <typename ... Targets>
  std::string moveValue(int current, Targets ... target) {
    // [->+<]
    return std::string("[-") + movePtr(current, target) + "+" + movePtr(target, current) + "]";
  }

  std::string copyValue(int current, int target, int tmp) {
    // [->+>+<<]>>[-<<+>>]
    std::ostringstream oss;
    oss << "[-" << movePtr(current, target) << "+" << movePtr(target, tmp) << "+" << movePtr(tmp, current) << "]"
	<< movePtr(current, tmp)
	<< "[-" << movePtr(tmp, current) << "+" << movePtr(current, tmp) << "]";
    return oss.str();
  }
  
}

int main() {
}
