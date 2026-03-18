#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

// ============================================================
// Minimal Brainfuck compiler test runner
//
// What this gives you:
//   - a tiny Brainfuck interpreter
//   - a lightweight test harness
//   - golden-output style integration tests
//   - optional "must halt" / "may loop" expectations
//
// Typical usage:
//   1. Replace the EXAMPLE TESTS section with your own tests.
//   2. In each test, build a Compiler program and return c.dumpBrainfuck().
//   3. Set the expected output string.
//   4. Run the executable.
//
// This file is intentionally dependency-free.
// ============================================================

namespace bftest {

// ============================================================
// Brainfuck interpreter
// ============================================================

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

    auto &cell = [&](long long ptr) -> std::uint8_t& {
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
                break; // Ignore non-BF characters.
        }
        ++pc;
    }

    result.halted = true;
    result.success = true;
    result.message = "ok";
    return result;
}

// ============================================================
// Test harness
// ============================================================

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

        bool haltOk = (tc.requireHalt == rr.halted) || (!tc.requireHalt && !rr.halted);
        bool outputOk = (tc.expectedOutput == rr.output);
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

inline int runAll(std::vector<TestCase> const &tests, bool failFast = false) {
    std::size_t passed = 0;

    for (auto const &tc : tests) {
        TestOutcome r = runTest(tc);

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

// ============================================================
// Convenience helpers for defining tests
// ============================================================

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

// ============================================================
// EXAMPLE TESTS
//
// Replace these with your own tests.
//
// Pattern:
//   - build a Compiler program
//   - return c.dumpBrainfuck()
//   - set expected output
//
// You can keep your old handwritten tests almost unchanged: just turn
// each old `mainXX()` into a lambda that returns the generated BF.
// ============================================================

#if __has_include("compiler.h")
#include "compiler.h"
#endif

int main() {
    std::vector<bftest::TestCase> tests;

    tests.push_back(expectOutput(
        "Global Variables 1"
        "AF",
        []() -> std::string {
#include "tests/global_variables_1.cc"
        }));

    tests.push_back(expectOutput(
        "Return Value i8",
        "X",
        []() {
#include "tests/return_value_i8.cc"
        }));

    tests.push_back(expectOutput(
        "Global Variable Return",
        "GG",
        []() {
#include "tests/global_variable_return.cc"
        }));

    tests.push_back(expectOutput(
        "Assign Return-Value to Global",
        "YY",
        []() {
#include "tests/assign_return_to_global.cc"
        }));

    tests.push_back(expectOutput(
        "Return i8 Array",
        "ABCDABCD",
        []() {
#include "tests/return_i8_array.cc"
        }));
    
    
    
        

    return bftest::runAll(tests);
}
