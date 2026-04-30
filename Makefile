# ---------- Config ----------
CXX      := g++
CXXFLAGS := -std=c++23 -Wall -Iinclude
LDFLAGS  :=
LDLIBS   :=

TARGET      := bcc
TEST_TARGET := bcc_tests
OBJDIR      := obj

# Shared library/application sources, without app/main.cc.
COMMON_SRCS := \
	src/api/api.cc \
	src/ir/primitive.cc \
	src/types/typesystem.cc \
	src/types/type_rules.cc \
	src/types/operators.cc \
	src/types/literal.cc \
	src/types/literal_impl.cc \
	src/types/literal_builders.cc \
	src/types/type_builders.cc \
	src/core/proxy.cc \
	src/core/slot.cc \
	src/assembler/assembler_builders.cc \
	src/assembler/assembler_program.cc \
	src/assembler/assembler_access.cc \
	src/assembler/assembler_calls_control.cc \
	src/assembler/assembler_pointers_memory.cc \
	src/assembler/assembler_algorithms.cc \
	src/assembler/assembler_codeblocks.cc \
	src/assembler/assembler_framenav.cc \
	src/assembler/assembler_diag.cc \
	src/assembler/assembler_codegen.cc \
	src/assembler/assembler_globals.cc \
	src/assembler/assembler_memory.cc \
	src/assembler/assembler_rlvalue.cc \
	src/assembler/assembler_unary.cc \
	src/assembler/assembler_add.cc \
	src/assembler/assembler_sub.cc \
	src/assembler/assembler_divmod.cc \
	src/assembler/assembler_mul.cc \
	src/assembler/assembler_logical.cc \
	src/assembler/assembler_comparisons.cc \
	src/assembler/assembler_binop_general.cc \

MAIN_SRC := app/main.cc
TEST_SRC := testsuite/testsuite.cc

EXAMPLE_SRCS    := $(filter-out %~,$(wildcard examples/*.cc))
EXAMPLE_TARGETS := $(patsubst examples/%.cc,examples/%,$(EXAMPLE_SRCS))
EXAMPLE_OBJS    := $(patsubst %.cc,$(OBJDIR)/%.o,$(EXAMPLE_SRCS))

COMMON_OBJS := $(patsubst %.cc,$(OBJDIR)/%.o,$(COMMON_SRCS))
MAIN_OBJ    := $(patsubst %.cc,$(OBJDIR)/%.o,$(MAIN_SRC))
TEST_OBJ    := $(patsubst %.cc,$(OBJDIR)/%.o,$(TEST_SRC))

$(TEST_OBJ): CXXFLAGS += -O3

DEPS := $(COMMON_OBJS:.o=.d) \
	$(MAIN_OBJ:.o=.d) \
	$(TEST_OBJ:.o=.d) \
	$(EXAMPLE_OBJS:.o=.d)

.PHONY: all
all: $(TARGET)

$(TARGET): $(COMMON_OBJS) $(MAIN_OBJ)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

.PHONY: tests
tests: $(TEST_TARGET)

$(TEST_TARGET): $(COMMON_OBJS) $(TEST_OBJ)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

.PHONY: examples
examples: $(EXAMPLE_TARGETS)

examples/%: $(COMMON_OBJS) $(OBJDIR)/examples/%.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJDIR)/%.o: %.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -MF $(@:.o=.d) -MT $@ -c $< -o $@

-include $(DEPS)

.PHONY: clean
clean:
	$(RM) -r $(OBJDIR) $(TARGET) $(TEST_TARGET) $(EXAMPLE_TARGETS)
