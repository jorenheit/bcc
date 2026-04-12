#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/wait.h>
#include <unordered_map>
#include <utility>
#include <vector>

// ============================================================
// Minimal Brainfuck compiler test runner
//
// This version runs each test in a subprocess by default.
// That isolates global/static state and makes cross-test
// interference much easier to avoid and debug.
//
// Typical usage:
//   - add tests to the vector in main()
//   - each test returns the generated Brainfuck program
//   - parent process launches this same executable with
//     --run-test <index> for each test case
// ============================================================

namespace bftest {

  struct RunOptions {
    std::size_t maxSteps = 50'000'000;
    bool errorOnPointerUnderflow = false;
  };

  struct RunResult {
    std::string output;
    bool halted = false;
    bool success = false;
    std::string message;
    std::size_t steps = 0;
  };

  inline std::vector<std::size_t> buildJumpTable(std::string const &program) {
    std::vector<std::size_t> jump(program.size(), static_cast<std::size_t>(-1));
    std::vector<std::size_t> stack;
    stack.reserve(program.size() / 4);

    for (std::size_t i = 0; i < program.size(); ++i) {
      if (program[i] == '[') {
	stack.push_back(i);
      } else if (program[i] == ']') {
	if (stack.empty()) {
	  throw std::runtime_error("Unmatched ']' at pc=" + std::to_string(i));
	}
	auto open = stack.back();
	stack.pop_back();
	jump[open] = i;
	jump[i] = open;
      }
    }

    if (!stack.empty()) {
      throw std::runtime_error("Unmatched '[' at pc=" + std::to_string(stack.back()));
    }

    return jump;
  }

  inline RunResult runBrainfuck(std::string const &program,
				std::string const &input = "",
				RunOptions opts = {}) {
    RunResult result;

    std::vector<std::size_t> jump;
    try {
      jump = buildJumpTable(program);
    } catch (std::exception const &e) {
      result.message = e.what();
      return result;
    }

    std::unordered_map<long long, std::uint8_t> tape;
    tape.reserve(4096);

    auto cell = [&](long long ptr) -> std::uint8_t& {
      return tape[ptr];
    };

    long long ptr = 0;
    std::size_t pc = 0;
    std::size_t inputPos = 0;

    while (pc < program.size()) {
      if (result.steps++ >= opts.maxSteps) {
	result.message = "Step limit exceeded";
	result.halted = false;
	result.success = false;
	return result;
      }

      switch (program[pc]) {
      case '>':
	++ptr;
	break;
      case '<':
	--ptr;
	if (opts.errorOnPointerUnderflow && ptr < 0) {
	  result.message = "Pointer moved below 0";
	  return result;
	}
	break;
      case '+':
	++cell(ptr);
	break;
      case '-':
	--cell(ptr);
	break;
      case '.':
	result.output.push_back(static_cast<char>(cell(ptr)));
	break;
      case ',':
	cell(ptr) = inputPos < input.size()
	  ? static_cast<std::uint8_t>(input[inputPos++])
	  : 0;
	break;
      case '[':
	if (cell(ptr) == 0) {
	  pc = jump[pc];
	}
	break;
      case ']':
	if (cell(ptr) != 0) {
	  pc = jump[pc];
	}
	break;
      default:
	break;
      }
      ++pc;
    }

    result.halted = true;
    result.success = true;
    result.message = "ok";
    return result;
  }

  struct TestCase {
    std::string name;
    std::function<std::string()> buildProgram;
    std::string expectedOutput;
    std::string input;
    bool requireHalt = true;
    std::size_t maxSteps = 50'000'000;
  };

  struct TestOutcome {
    std::string name;
    bool passed = false;
    std::string expectedOutput;
    std::string actualOutput;
    bool expectedHalt = true;
    bool actualHalt = false;
    std::string message;
    std::size_t steps = 0;
  };

  inline std::string hexEncode(std::string const &s) {
    static constexpr char digits[] = "0123456789ABCDEF";
    std::string out;
    out.reserve(s.size() * 2);
    for (unsigned char c : s) {
      out.push_back(digits[c >> 4]);
      out.push_back(digits[c & 0x0F]);
    }
    return out;
  }

  inline std::string hexDecode(std::string const &s) {
    auto nibble = [](char c) -> int {
      if (c >= '0' && c <= '9') return c - '0';
      if (c >= 'A' && c <= 'F') return c - 'A' + 10;
      if (c >= 'a' && c <= 'f') return c - 'a' + 10;
      throw std::runtime_error("Invalid hex digit");
    };

    if (s.size() % 2 != 0) {
      throw std::runtime_error("Invalid hex length");
    }

    std::string out;
    out.reserve(s.size() / 2);
    for (std::size_t i = 0; i < s.size(); i += 2) {
      out.push_back(static_cast<char>((nibble(s[i]) << 4) | nibble(s[i + 1])));
    }
    return out;
  }

  inline bool startsWith(std::string const &s, std::string const &prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
  }

  inline TestOutcome runTest(TestCase const &tc) {
    TestOutcome out;
    out.name = tc.name;
    out.expectedOutput = tc.expectedOutput;
    out.expectedHalt = tc.requireHalt;

    try {
      std::string bf = tc.buildProgram();
      RunResult rr = runBrainfuck(bf, tc.input, RunOptions{.maxSteps = tc.maxSteps});

      out.actualOutput = rr.output;
      out.actualHalt = rr.halted;
      out.message = rr.message;
      out.steps = rr.steps;

      bool haltOk = (tc.requireHalt == rr.halted);
      bool outputOk = tc.requireHalt
	? (tc.expectedOutput == rr.output)
	: startsWith(rr.output, tc.expectedOutput);
      out.passed = haltOk && outputOk;
    } catch (std::exception const &e) {
      out.actualOutput.clear();
      out.actualHalt = false;
      out.message = std::string("Exception: ") + e.what();
      out.passed = false;
    } catch (...) {
      out.actualOutput.clear();
      out.actualHalt = false;
      out.message = "Unknown exception";
      out.passed = false;
    }

    return out;
  }

  inline void emitSingleTestResult(TestOutcome const &r) {
    std::cout << "BFT_RESULT_BEGIN\n";
    std::cout << "name=" << hexEncode(r.name) << "\n";
    std::cout << "passed=" << (r.passed ? 1 : 0) << "\n";
    std::cout << "expectedOutput=" << hexEncode(r.expectedOutput) << "\n";
    std::cout << "actualOutput=" << hexEncode(r.actualOutput) << "\n";
    std::cout << "expectedHalt=" << (r.expectedHalt ? 1 : 0) << "\n";
    std::cout << "actualHalt=" << (r.actualHalt ? 1 : 0) << "\n";
    std::cout << "message=" << hexEncode(r.message) << "\n";
    std::cout << "steps=" << r.steps << "\n";
    std::cout << "BFT_RESULT_END\n";
  }

  inline std::string readAllFromFd(int fd) {
    std::string out;
    char buffer[4096];
    while (true) {
      ssize_t n = ::read(fd, buffer, sizeof(buffer));
      if (n == 0) break;
      if (n < 0) throw std::runtime_error("Failed to read from child pipe");
      out.append(buffer, static_cast<std::size_t>(n));
    }
    return out;
  }

  inline TestOutcome parseSingleTestResult(std::string const &blob) {
    std::istringstream iss(blob);
    std::string line;
    std::unordered_map<std::string, std::string> kv;
    bool sawBegin = false;
    bool sawEnd = false;

    while (std::getline(iss, line)) {
      if (line == "BFT_RESULT_BEGIN") {
	sawBegin = true;
	continue;
      }
      if (line == "BFT_RESULT_END") {
	sawEnd = true;
	break;
      }
      auto pos = line.find('=');
      if (pos != std::string::npos) {
	kv.emplace(line.substr(0, pos), line.substr(pos + 1));
      }
    }

    if (!sawBegin || !sawEnd) {
      throw std::runtime_error("Could not parse child test result");
    }

    TestOutcome r;
    r.name = hexDecode(kv.at("name"));
    r.passed = (kv.at("passed") == "1");
    r.expectedOutput = hexDecode(kv.at("expectedOutput"));
    r.actualOutput = hexDecode(kv.at("actualOutput"));
    r.expectedHalt = (kv.at("expectedHalt") == "1");
    r.actualHalt = (kv.at("actualHalt") == "1");
    r.message = hexDecode(kv.at("message"));
    r.steps = static_cast<std::size_t>(std::stoull(kv.at("steps")));
    return r;
  }

  inline TestOutcome runSingleInSubprocess(std::vector<TestCase> const &tests,
					   std::size_t index,
					   char const *selfPath) {
    if (index >= tests.size()) {
      throw std::runtime_error("Test index out of range");
    }

    int pipeFds[2];
    if (::pipe(pipeFds) != 0) {
      throw std::runtime_error("pipe() failed");
    }

    pid_t pid = ::fork();
    if (pid < 0) {
      ::close(pipeFds[0]);
      ::close(pipeFds[1]);
      throw std::runtime_error("fork() failed");
    }

    if (pid == 0) {
      ::close(pipeFds[0]);
      if (::dup2(pipeFds[1], STDOUT_FILENO) < 0) {
	std::perror("dup2 stdout failed");
	std::_Exit(121);
      }
      if (::dup2(pipeFds[1], STDERR_FILENO) < 0) {
	std::perror("dup2 stderr failed");
	std::_Exit(122);
      }
      ::close(pipeFds[1]);

      std::string indexStr = std::to_string(index);
      char *const argv[] = {
	const_cast<char *>(selfPath),
	const_cast<char *>("--run-test"),
	indexStr.data(),
	nullptr
      };
      ::execv(selfPath, argv);
      std::perror("execv failed");
      std::_Exit(123);
    }

    ::close(pipeFds[1]);
    std::string childOutput = readAllFromFd(pipeFds[0]);
    ::close(pipeFds[0]);

    int status = 0;
    if (::waitpid(pid, &status, 0) < 0) {
      throw std::runtime_error("waitpid() failed");
    }

    TestOutcome parsed;
    try {
      parsed = parseSingleTestResult(childOutput);
    } catch (std::exception const &e) {
      parsed.name = tests[index].name;
      parsed.expectedOutput = tests[index].expectedOutput;
      parsed.expectedHalt = tests[index].requireHalt;
      parsed.actualHalt = false;
      parsed.passed = false;
      parsed.message = std::string("Child crashed or produced unreadable output: ") + e.what() +
	"\n--- child output ---\n" + childOutput;
    }

    if (WIFSIGNALED(status)) {
      parsed.passed = false;
      parsed.actualHalt = false;
      parsed.message += std::string(parsed.message.empty() ? "" : "\n") +
	"Child terminated by signal " + std::to_string(WTERMSIG(status));
    } else if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
      parsed.passed = false;
      parsed.message += std::string(parsed.message.empty() ? "" : "\n") +
	"Child exited with code " + std::to_string(WEXITSTATUS(status));
    }

    return parsed;
  }

  inline int runAllIsolated(std::vector<TestCase> const &tests,
			    char const *selfPath,
			    bool failFast = false) {
    std::size_t passed = 0;

    for (std::size_t i = 0; i < tests.size(); ++i) {
      TestOutcome r = runSingleInSubprocess(tests, i, selfPath);

      std::cout << (r.passed ? "[PASS] " : "[FAIL] ") << r.name;
      if (!r.passed) {
	std::cout << "\n"
		  << "  expected output: " << std::quoted(r.expectedOutput) << "\n"
		  << "  actual output:   " << std::quoted(r.actualOutput) << "\n"
		  << "  expected halt:   " << (r.expectedHalt ? "yes" : "no") << "\n"
		  << "  actual halt:     " << (r.actualHalt ? "yes" : "no") << "\n"
		  << "  message:         " << r.message << "\n"
		  << "  steps:           " << r.steps;
      }
      std::cout << "\n";

      if (r.passed) {
	++passed;
      } else if (failFast) {
	break;
      }
    }

    std::cout << "\nSummary: " << passed << "/" << tests.size() << " passed\n";
    return passed == tests.size() ? EXIT_SUCCESS : EXIT_FAILURE;
  }

} // namespace bftest

inline bftest::TestCase expectOutput(std::string name,
                                     std::string expectedOutput,
                                     std::function<std::string()> buildProgram,
                                     std::size_t maxSteps = 50'000'000) {
  return bftest::TestCase{
    .name = std::move(name),
    .buildProgram = std::move(buildProgram),
    .expectedOutput = std::move(expectedOutput),
    .input = "",
    .requireHalt = true,
    .maxSteps = maxSteps,
  };
}

inline bftest::TestCase expectLoopingOutput(std::string name,
                                            std::string expectedPrefixOutput,
                                            std::function<std::string()> buildProgram,
                                            std::size_t maxSteps = 2'000'000) {
  return bftest::TestCase{
    .name = std::move(name),
    .buildProgram = std::move(buildProgram),
    .expectedOutput = std::move(expectedPrefixOutput),
    .input = "",
    .requireHalt = false,
    .maxSteps = maxSteps,
  };
}

#include "../compiler.h"
#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

#define TEST_BEGIN Compiler c; c.setEntryPoint("main"); c.begin();
#define TEST_END c.end(); return c.dumpBrainfuck();

static std::vector<bftest::TestCase> buildTests() {
  std::vector<bftest::TestCase> tests;
  tests.reserve(128);  
    
  tests.push_back(expectOutput("Global Variables 1",
			       "AF", []() {
#include "tests/global_variables_1.cc"
			       }));

  tests.push_back(expectOutput("Return Value i8",
			       "X", []() {
#include "tests/return_value_i8.cc"
			       }));

  tests.push_back(expectOutput("Global Variable Return",
			       "GG", []() {
#include "tests/global_variable_return.cc"
			       }));

  tests.push_back(expectOutput("Assign Return-Value to Global",
			       "YY",[]() {
#include "tests/assign_return_to_global.cc"
			       }));

  tests.push_back(expectOutput("Return i8 Array",
			       "ABCDABCD", []() {
#include "tests/return_i8_array.cc"
			       }));

  tests.push_back(expectOutput("Return i16 Array",
			       "ABCDEFGH", []() {
#include "tests/return_i16_array.cc"
			       }));

  tests.push_back(expectOutput("i8 Function Arguments",
			       "AB", []() {
#include "tests/function_arg_i8.cc"
			       }));

  tests.push_back(expectOutput("i16 Function Arguments",
			       "AB", []() {
#include "tests/function_arg_i16.cc"
			       }));
  
  tests.push_back(expectOutput("i8 Array Function Argument",
			       "ABCDABCD", []() {
#include "tests/function_arg_i8_array.cc"
			       }));

  tests.push_back(expectOutput("i16 Array Function Argument",
			       "ABCDABCD", []() {
#include "tests/function_arg_i16_array.cc"
			       }));
  
  tests.push_back(expectOutput("Global i8 Function Argument",
			       "GHG", []() {
#include "tests/function_arg_i8_global.cc"
			       }));

  tests.push_back(expectOutput("Mixed Function Arguments",
			       "ABCDEF", []() {
#include "tests/function_arg_mixed.cc"
			       }));

  tests.push_back(expectOutput("Nested Function Calls",
			       "A", []() {
#include "tests/function_call_nested.cc"
			       }));


  tests.push_back(expectLoopingOutput("Recursive Function Calls",
				      "ABC", []() {
#include "tests/recursive_call_with_arguments.cc"
				      }));

  tests.push_back(expectOutput("Function Call with Variable and Constants",
			       "AZBY", []() {
#include "tests/function_call_with_constants.cc"
			       }));

  tests.push_back(expectOutput("Nested Function Call with Return Value",
			       "Z", []() {
#include "tests/function_call_nested_with_return.cc"
			       }));
  
  tests.push_back(expectOutput("Function Call with Anonymous Array",
			       "ABCD", []() {
#include "tests/function_call_with_anonymous_array.cc"
			       }));

  tests.push_back(expectOutput("Function Call with Nested Anonymous Array",
			       "ABCD", []() {
#include "tests/function_call_with_anonymous_nested_array.cc"
			       }));

  tests.push_back(expectOutput("Function Call with Anonymous Array Containing Vars",
			       "AB", []() {
#include "tests/function_call_with_anonymous_array_containing_vars.cc"
			       }));

  tests.push_back(expectOutput("Passing Arrays of Arrays Mixed Init",
			       "ABCDE", []() {
#include "tests/passing_arrays_of_arrays_mixed_init.cc"
			       }));

  tests.push_back(expectOutput("Shadowing Local Variables in Nested Scopes",
			       "ABXCDA", []() {
#include "tests/shadowing_local_variables.cc"
			       }));

  tests.push_back(expectOutput("WriteOut Temp i8",
			       "A", []() {
#include "tests/writeout_with_temporary_cell.cc"
			       }));

  tests.push_back(expectOutput("WriteOut Temp i16",
			       "AB", []() {
#include "tests/writeout_with_temporary_cell_i16.cc"
			       }));

  tests.push_back(expectOutput("WriteOut Temp Array",
			       "ABCD", []() {
#include "tests/writeout_with_temporary_array.cc"
			       }));

  tests.push_back(expectOutput("Passing String Variable of Same Size",
			       "Hello World", []() {
#include "tests/passing_string_as_variable_same_size.cc"
			       }));

  tests.push_back(expectOutput("Passing String Anonymously of Same Size",
			       "Hello World", []() {
#include "tests/passing_string_anonymous_same_size.cc"
			       }));

  tests.push_back(expectOutput("Passing String Variable of Smaller Size",
			       "Hello World", []() {
#include "tests/passing_string_variable_smaller_size.cc"
			       }));

  tests.push_back(expectOutput("Passing String Anonymous of Smaller Size",
			       "Hello World", []() {
#include "tests/passing_string_anonymous_smaller_size.cc"
			       }));

  tests.push_back(expectOutput("i8 to i16",
			       std::string("AA") + '\0', []() {
#include "tests/i8_to_i16_conversion.cc"
			       }));

  tests.push_back(expectOutput("Reading and Writing Struct Fields",
			       "ABAB", []() {
#include "tests/struct_field_read_write.cc"
			       }));

  tests.push_back(expectOutput("Mutating Global Struct",
			       "ABAC", []() {
#include "tests/reading_and_writing_from_global_struct.cc"
			       }));

  tests.push_back(expectOutput("Return Value to Field",
			       "AZ", []() {
#include "tests/return_function_value_into_struct_field.cc"
			       }));

  tests.push_back(expectOutput("Passing Struct to Function",
			       "ABABXYAB", []() {
#include "tests/passing_struct_to_function.cc"
			       }));

  tests.push_back(expectOutput("Return Struct from Function",
			       "QR", []() {
#include "tests/return_struct_from_function.cc"
			       }));

  tests.push_back(expectOutput("Assign Anonymous Struct",
			       "AB", []() {
#include "tests/assign_anonymous_struct.cc"
			       }));

  tests.push_back(expectOutput("Print Anonymous Struct",
			       "AB", []() {
#include "tests/print_anonymous_struct.cc"
			       }));
  
  tests.push_back(expectOutput("Assign Anonymous To Dynamic Array Element",
			       "ABZD", []() {
#include "tests/assign_anonymous_to_dynamic_array_element.cc"
			       }));  

  tests.push_back(expectOutput("Assign Runtime Value To Dynamic Array Element",
			       "AYCD", []() {
#include "tests/assign_runtime_value_to_dynamic_array_element.cc"
			       }));

  tests.push_back(expectOutput("Dynamic Nested Array Element Write",
			       "ABXD", []() {
#include "tests/dynamic_nested_array_element_write.cc"
			       }));

  tests.push_back(expectOutput("Dynamic Array Of Structs Field Write",
			       "ABQRCD", []() {
#include "tests/dynamic_array_of_structs_field_write.cc"
			       }));

  tests.push_back(expectOutput("Return Value Into Dynamic Array Slot",
			       "ABCZ", []() {
#include "tests/return_value_into_dynamic_array_slot.cc"
			       }));

  tests.push_back(expectOutput("Return Value Into Dynamic Nested Struct Field",
			       "MNOQ", []() {
#include "tests/return_value_into_dynamic_nested_struct_field.cc"
			       })); 


tests.push_back(expectOutput("Read Dynamic Array Element Into Local",
                             "C", []() {
#include "tests/read_dynamic_array_element_into_local.cc"
                             }));

 tests.push_back(expectOutput("Assign Dynamic Array Element From Dynamic Array Element",
                             "ABAD", []() {
#include "tests/assign_dynamic_array_element_from_dynamic_array_element.cc"
                             }));

 tests.push_back(expectOutput("Self Assign Dynamic Array Element",
                             "ABCD", []() {
#include "tests/self_assign_dynamic_array_element.cc"
                             }));

 tests.push_back(expectOutput("Dynamic Array Element Boundary First",
                             "ZBCD", []() {
#include "tests/dynamic_array_element_boundary_first.cc"
                             }));
 
 tests.push_back(expectOutput("Dynamic Array Element Boundary Last",
                             "ABCZ", []() {
#include "tests/dynamic_array_element_boundary_last.cc"
                             }));

tests.push_back(expectOutput("Struct Field Array Dynamic Write",
                             "AQCD", []() {
#include "tests/struct_field_array_dynamic_write.cc"
                             }));

 tests.push_back(expectOutput("Read Dynamic Array Of Structs Field Into Local",
                             "C", []() {
#include "tests/read_dynamic_array_of_structs_field_into_local.cc"
                             }));

 tests.push_back(expectOutput("Pass Dynamic Array Element As Argument",
                             "C", []() {
#include "tests/pass_dynamic_array_element_as_argument.cc"
                             }));

 tests.push_back(expectOutput("Reuse Dynamic Array Element After Call",
                             "X", []() {
#include "tests/reuse_dynamic_array_element_after_call.cc"
                             }));

 tests.push_back(expectOutput("Static And Dynamic Nested Array Access",
                             "ABQD", []() {
#include "tests/static_and_dynamic_nested_array_access.cc"
                             }));

 tests.push_back(expectOutput("Reading from a global pointer inside function",
                             "GHX", []() {
#include "tests/reading_global_pointer_from_function.cc"
                             }));

 tests.push_back(expectOutput("Reading and writing through a local pointer",
			      "XXYX", []() {
#include "tests/reading_and_writing_local_pointer.cc"
			      }));

tests.push_back(expectOutput("Reading from a local pointer inside function",
                             "XX", []() {
#include "tests/reading_local_pointer_from_function.cc"
                             }));

 tests.push_back(expectOutput("Writing through a local pointer inside function",
			      "YY", []() {
#include "tests/writing_local_pointer_from_function.cc"
			      }));

 tests.push_back(expectOutput("Writing through a global pointer inside function",
			      "HH", []() {
#include "tests/writing_global_pointer_from_function.cc"
			      }));

 tests.push_back(expectOutput("Array of local pointers",
			      "ABXY", []() {
#include "tests/array_of_local_pointers.cc"
			      }));

 tests.push_back(expectOutput("Array of passed pointers",
			      "ABXY", []() {
#include "tests/array_of_passed_pointers.cc"
			      }));

 return tests;
}

int main(int argc, char **argv) {
  auto tests = buildTests();

  if (argc == 3 && std::strcmp(argv[1], "--run-test") == 0) {
    std::size_t index = static_cast<std::size_t>(std::stoull(argv[2]));
    if (index >= tests.size()) {
      std::cerr << "Test index out of range\n";
      return 2;
    }
    auto outcome = bftest::runTest(tests[index]);
    bftest::emitSingleTestResult(outcome);
    return outcome.passed ? 0 : 1;
  }

  return bftest::runAllIsolated(tests, argv[0]);
}
